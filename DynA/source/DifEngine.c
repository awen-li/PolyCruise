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



static inline VOID Insert2FDif (DWORD Handle, ULONG FID, Node *N)
{
    DbReq Req;
    DbAck Ack;

    Req.dwDataType = Handle;
    Req.dwKeyLen   = sizeof (ULONG);
    Req.pKeyCtx    = (BYTE*)(&FID);
    
    DWORD Ret = CreateDataByKey (&Req, &Ack);
    assert (Ret == R_SUCCESS);

    List *FDL = (List *)(Ack.pDataAddr);
    ListInsert (FDL, N);

    return;
}

static inline VOID DelDifNode (Node *GN)
{
    DifNode* DifN = GN_2_DIFN (GN);

    DelEventMsg (DifN->EMsg);
    DifN->EMsg = NULL;

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

    Node *N    = (Node *)(Ack.pDataAddr);
    N->EventID = Event;

    AddNode (DifGraph, N);

    return N;
    //return GN_2_DIFN (N);
}


static inline Edge* AddDifEdge (Node *S, Node *D)
{
    DbReq Req;
    DbAck Ack;

    Graph *DifGraph = DifA.DifGraph;
    assert (DifGraph != NULL);

    Edge EC = {S, D, NULL};
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



static inline VOID InsertNode2Graph (Graph *DifGraph, Node *N)
{   


    return;
}

VOID DifEngine (ULONG Event, char *Msg)
{
    Graph *DifGraph = DifA.DifGraph;
    printf ("[DIF]%lx: %s \r\n", Event, Msg);

    EventMsg *EMsg = DecodeEventMsg (Event, Msg);
    ViewEMsg (EMsg);

    Node *N = AddDifNode (Event);
    DifNode* DifN = GN_2_DIFN (N); 
    DifN->EMsg = EMsg;

    InsertNode2Graph (DifGraph, N);
    
    if (R_EID2IID(EMsg->EventId) != 0)
    {
    }
    else
    {
             
    }
    

    return;
}




