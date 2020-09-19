/***********************************************************
 * Author: Wen Li
 * Date  : 9/9/2020
 * Describe: DifEngine.c  
 * History:
   <1> 9/9/2020 , create
************************************************************/
#include "MacroDef.h"
#include "DifGraph.h"

static DifAgent DifA;

Graph *GetDIFG ()
{
    DifAgent *DA = &DifA;

    return DA->DifGraph;    
}

VOID InitDif ()
{
    DifAgent *DA = &DifA;
    
    if (DA->DifGraph == NULL)
    {
        DA->DifGraph = CreateGraph (DB_TYPE_DIF_NODE, DB_TYPE_DIF_EDGE);
        assert (DA->DifGraph != NULL);
    }

    DWORD Ret;
    Graph *DifGraph = DA->DifGraph;
    
    Ret = DbCreateTable(DifGraph->NDBType, sizeof (Node)+sizeof (DifNode), sizeof (ULONG));
    assert (Ret != R_FAIL);

    Ret = DbCreateTable(DifGraph->EDBType, sizeof (Edge)+sizeof (DifEdge), sizeof (Edge));
    assert (Ret != R_FAIL);

    DA->FDifHandle = DB_TYPE_DIF_FUNC;
    Ret = DbCreateTable(DA->FDifHandle, sizeof(List), sizeof (ULONG));
    assert (Ret != R_FAIL);

    //DA->DefHandle = DB_TYPE_DIF_DEF;
    //Ret = DbCreateTable(DA->DefHandle, sizeof (DNLNode), FUNC_NAME_LEN));
    //assert (Ret != R_FAIL);

    return;
}


static inline List* GetFDifG (DWORD Handle, ULONG FID)
{
    DbReq Req;
    DbAck Ack;

    Req.dwDataType = Handle;
    Req.dwKeyLen   = sizeof (ULONG);
    Req.pKeyCtx    = (BYTE*)(&FID);
    
    DWORD Ret = QueryDataByKey (&Req, &Ack);
    if (Ret != R_SUCCESS)
    {
        return NULL;
    }

    return (List *)(Ack.pDataAddr);
}



static inline List* CreateFDifG (DWORD Handle, ULONG FID)
{
    DbReq Req;
    DbAck Ack;

    Req.dwDataType = Handle;
    Req.dwKeyLen   = sizeof (ULONG);
    Req.pKeyCtx    = (BYTE*)(&FID);
    
    DWORD Ret = CreateDataByKey (&Req, &Ack);
    assert (Ret == R_SUCCESS);

    return (List *)(Ack.pDataAddr);
}

static inline VOID DelDifNode (Node *GN)
{
    DifNode* DifN = GN_2_DIFN (GN);

    DelEventMsg (&DifN->EMsg);

    memset (DifN, 0, sizeof (DifNode));
    return;        
}


VOID DeInitDif ()
{
    DifAgent *DA = &DifA;
    
    if (DA->DifGraph != NULL)
    {
        ListVisit (&DA->DifGraph->NodeList, (ProcData)DelDifNode);
        
        DelGraph (DA->DifGraph);
        DA->DifGraph = NULL;
    }

    DelDb ();

    return;
}


DWORD IsEventExist (ULONG Event)
{
    DbReq Req;
    DbAck Ack;

    Graph *DifGraph = DifA.DifGraph;
    assert (DifGraph != NULL);

    Req.dwDataType = DifGraph->NDBType;
    Req.dwKeyLen   = sizeof (ULONG);
    Req.pKeyCtx    = (BYTE*)(&Event);

    Ack.dwDataId = 0;
    (VOID)QueryDataByKey(&Req, &Ack);
    if (Ack.dwDataId != 0)
    {
        return TRUE;
    }

    return FALSE;
}

static inline Node* AddDifNode (ULONG Event)
{
    DbReq Req;
    DbAck Ack;

    Graph *DifGraph = DifA.DifGraph;
    assert (DifGraph != NULL);

    Req.dwDataType = DifGraph->NDBType;
    Req.dwKeyLen   = sizeof (ULONG);
    Req.pKeyCtx    = (BYTE*)(&Event);
    
    DWORD Ret = CreateDataByKey (&Req, &Ack);
    assert (Ret == R_SUCCESS);

    Node *N = (Node *)(Ack.pDataAddr);
    N->Id   = Ack.dwDataId;

    return N;
}


static inline Edge* AddDifEdge (Node *S, Node *D)
{
    DbReq Req;
    DbAck Ack;

    Graph *DifGraph = DifA.DifGraph;
    assert (DifGraph != NULL);

    Edge EC = {S, D};
    Req.dwDataType = DifGraph->EDBType;
    Req.dwKeyLen   = sizeof (Edge);
    Req.pKeyCtx    = (BYTE*)(&EC);
    
    DWORD Ret = CreateDataByKey (&Req, &Ack);
    assert (Ret == R_SUCCESS);

    Edge *E = (Edge *)(Ack.pDataAddr);
    E->Src = S;
    E->Dst = D;
    AddEdge(DifGraph, E);

    return E;
}


static inline DWORD IsNodeDD (DifNode *DN, EventMsg * Emsg)
{
    LNode *Def = DN->EMsg.Def.Header;
    while (Def != NULL)
    {
        Variable *DV = (Variable *)Def->Data;
        assert (DV != NULL);

        if (DV->Type == VT_FUNCTION)
        {
            Def = Def->Nxt;
            continue;
        }

        printf ("===>Definition: %s \r\n", DV->Name);
        
        LNode *Use = Emsg->Use.Header;
        while (Use != NULL)
        {
            Variable *UV = (Variable *)Use->Data;
            assert (UV != NULL);

            printf ("===>Use: %s \r\n", UV->Name);
            if (strcmp (UV->Name, DV->Name) == 0 /*|| UV->Addr == DV->Addr */ )
            {
                return TRUE;
            }
            
            Use = Use->Nxt;
        }

        Def = Def->Nxt;
        
    }
    
    return FALSE;
}


static inline VOID SetEdgeType (Edge* E, DWORD EType)
{
    DifEdge *DE = GE_2_DIFE (E);
    DE->EdgeType = EType;

    return;
}


static inline VOID InsertNode2Graph (Graph *DifGraph, Node *N)
{
    DifNode* DifN = GN_2_DIFN (N);
    DWORD FID = R_EID2FID (DifN->EventId);
    
    List *FDifG = GetFDifG (DB_TYPE_DIF_FUNC, FID);
    if (FDifG == NULL)
    {
        assert (R_EID2IID (DifN->EventId) == 0);
        FDifG = CreateFDifG (DB_TYPE_DIF_FUNC, FID);

        Node *LN = GetLastNode (DifGraph);
        if (LN != NULL)
        {
            Edge* E = AddDifEdge (LN, N);
            SetEdgeType (E, EDGE_CF);
        }
    }
    else
    {
        EventMsg *CurMsg = &DifN->EMsg;
            
        LNode *LN = FDifG->Tail;
        DWORD ExistDD = FALSE;
        while (LN != NULL)
        {
            Node *TempN = (Node *)LN->Data;

            ExistDD = IsNodeDD (GN_2_DIFN (TempN), CurMsg);
            if (ExistDD == TRUE)
            {
                break;
            }

            LN = LN->Pre;        
        }

        if (ExistDD == TRUE)
        {
            Edge* E = AddDifEdge ((Node *)LN->Data, N);
            SetEdgeType (E, EDGE_DIF);
        }
        else
        {
            LNode *LnHdr = FDifG->Header;
            
            Edge* E = AddDifEdge ((Node *)LnHdr->Data, N);
            SetEdgeType (E, EDGE_CF);
        }
    }
  
    ListInsert (FDifG, N);
    AddNode (DifGraph, N);

    return;
}

VOID DifEngine (ULONG Event, char *Msg)
{
    Graph *DifGraph = DifA.DifGraph;
    printf ("[DIF]%lx: %s \r\n", Event, Msg);

    Node *N = AddDifNode (Event);
    DifNode* DifN = GN_2_DIFN (N);
    DifN->EventId = Event;

    DecodeEventMsg (&DifN->EMsg, Event, Msg);
    ViewEMsg (&DifN->EMsg);

    InsertNode2Graph (DifGraph, N);
    
    return;
}




