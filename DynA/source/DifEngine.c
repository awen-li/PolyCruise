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
    DWORD Ret;
    DifAgent *DA = &DifA;

    DA->NodeHandle = DB_TYPE_DIF_NODE;
    DA->EdgeHandle = DB_TYPE_DIF_EDGE;
    DA->FDifHandle = DB_TYPE_DIF_FUNC;
    DA->ThrHandle  = DB_TYPE_DIF_THR;
    
    Ret = DbCreateTable(DA->NodeHandle, sizeof (Node)+sizeof (DifNode), sizeof (EventKey));
    assert (Ret != R_FAIL);

    Ret = DbCreateTable(DA->EdgeHandle, sizeof (Edge)+sizeof (DifEdge), sizeof (Edge));
    assert (Ret != R_FAIL);
    
    Ret = DbCreateTable(DA->FDifHandle, sizeof(List), sizeof (EventKey));
    assert (Ret != R_FAIL);
    
    Ret = DbCreateTable(DA->ThrHandle, sizeof (Graph), sizeof (DWORD));
    assert (Ret != R_FAIL);

    return;
}


static inline List* GetFDifG (DWORD Handle, ULONG FID, DWORD ThreadId)
{
    DbReq Req;
    DbAck Ack;
    EventKey Ek = {0};

    Req.dwDataType = Handle;
    Req.dwKeyLen   = sizeof (EventKey);

    Ek.Event    = FID;
    Ek.ThreadId = ThreadId;
    Req.pKeyCtx = (BYTE *)(&Ek);
    
    DWORD Ret = QueryDataByKey (&Req, &Ack);
    if (Ret != R_SUCCESS)
    {
        return NULL;
    }

    return (List *)(Ack.pDataAddr);
}



static inline List* CreateFDifG (DWORD Handle, ULONG FID, DWORD ThreadId)
{
    DbReq Req;
    DbAck Ack;
    EventKey Ek = {0};

    Req.dwDataType = Handle;
    Req.dwKeyLen   = sizeof (EventKey);

    Ek.Event    = FID;
    Ek.ThreadId = ThreadId;
    Req.pKeyCtx = (BYTE *)(&Ek);
    
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

DWORD GetGraphNodeNum ()
{
    if (DifA.NodeHandle == 0)
    {
        return 0;
    }
    
    return QueryDataNum(DifA.NodeHandle);
}


Node *GetGraphNodeById (DWORD GraphNodeId)
{
    DWORD Ret;
    DbReq Req;
    DbAck Ack;
    Graph *G;
    
    if (DifA.NodeHandle == 0)
    {
        return 0;
    }

    Req.dwDataType = DifA.NodeHandle;
    Req.dwDataId   = GraphNodeId;

    Ack.dwDataId = 0;
    (VOID)QueryDataByID (&Req, &Ack);
    if (Ack.dwDataId == 0)
    {
        return NULL;
    }
    
    if (Ack.dwDataId != GraphNodeId)
    {
        DEBUG ("Data not consistent: IN: %d, OUT: %d\r\n", GraphNodeId, Ack.dwDataId);
        return NULL;
    }

    return (Node *)(Ack.pDataAddr);
}



VOID DeInitDif ()
{
    DifAgent *DA = &DifA;

    DWORD GraphNum = GetGraphNum ();
    DEBUG ("GraphNum: %u \r\n", GraphNum);

    DWORD GraphId = 1;
    while (GraphId <= GraphNum)
    {
        Graph *G = GetGraphById (GraphId);
        DEBUG ("[G%u]ThreadId: %x \r\n", GraphId, G->ThreadId);

        ListVisit (&G->NodeList, (ProcData)DelDifNode);
        
        DelGraph (G);

        GraphId++;
    }

    DelDb ();

    return;
}


static inline DWORD IsEventExist (Graph *DifGraph, ULONG Event, DWORD ThreadId)
{
    DbReq Req;
    DbAck Ack;
    EventKey Ek = {0};

    Req.dwDataType = DifGraph->NDBType;
    Req.dwKeyLen   = sizeof (EventKey);

    Ek.Event    = Event;
    Ek.ThreadId = ThreadId;
    Req.pKeyCtx = (BYTE *)(&Ek);

    Ack.dwDataId = 0;
    (VOID)QueryDataByKey(&Req, &Ack);
    if (Ack.dwDataId != 0)
    {
        return TRUE;
    }

    return FALSE;
}

static inline Node* AddDifNode (Graph *DifGraph, ULONG Event)
{
    DbReq Req;
    DbAck Ack;
    EventKey Ek = {0};

    Req.dwDataType = DifGraph->NDBType;
    Req.dwKeyLen   = sizeof (EventKey);

    Ek.Event    = Event;
    Ek.ThreadId = DifGraph->ThreadId;
    Req.pKeyCtx = (BYTE *)(&Ek);
    
    DWORD Ret = CreateDataByKey (&Req, &Ack);
    assert (Ret == R_SUCCESS);

    Node *N = (Node *)(Ack.pDataAddr);
    N->Id   = Ack.dwDataId;

    return N;
}


static inline Edge* AddDifEdge (Graph *DifGraph, Node *S, Node *D)
{
    DbReq Req;
    DbAck Ack;

    Edge EC = {S, D};
    Req.dwDataType = DifGraph->EDBType;
    Req.dwKeyLen   = sizeof (Edge);
    Req.pKeyCtx    = (BYTE*)(&EC);

    // Query first
    Ack.dwDataId = 0;
    (VOID)QueryDataByKey(&Req, &Ack);
    if (Ack.dwDataId == 0)
    {
        DWORD Ret = CreateDataByKey (&Req, &Ack);
        assert (Ret == R_SUCCESS);

        Edge *E = (Edge *)(Ack.pDataAddr);
        E->Src = S;
        E->Dst = D;
        AddEdge(DifGraph, E);

        return E;
    }
    else
    {
        return (Edge *)(Ack.pDataAddr);
    }
}


static inline DWORD IsNodeDD (DifNode *CurNode, DifNode *PreNode, UseMap *Um)
{
    DWORD UseNo;
    LNode *Def = PreNode->EMsg.Def.Header;
    while (Def != NULL)
    {
        Variable *DV = (Variable *)Def->Data;
        assert (DV != NULL);

        if (DV->Type == VT_FUNCTION)
        {
            Def = Def->Nxt;
            continue;
        }

        DEBUG ("===>Definition: %s \r\n", DV->Name);

        UseNo = 0;
        LNode *Use = CurNode->EMsg.Use.Header;
        while (Use != NULL)
        {
            Variable *UV = (Variable *)Use->Data;
            assert (UV != NULL);

            DEBUG ("===>Use: %s \r\n", UV->Name);
            if (Um->UseMap[UseNo] == 0 &&
                strcmp (UV->Name, DV->Name) == 0)
            {
                Um->UseMap[UseNo] = 1;
                Um->MapNum++;
                return TRUE;
            }
            
            Use = Use->Nxt;
            UseNo++;
        }

        Def = Def->Nxt;
        
    }
    
    return FALSE;
}


static inline VOID SetEdgeType (Edge* E, DWORD EType)
{
    DifEdge *DE = GE_2_DIFE (E);
    DE->EdgeType |= EType;

    return;
}


static inline VOID AddCallEdge (Graph *DifGraph, Node *LastNd, Node *CurNd)
{
    DifNode* LastDifN = GN_2_DIFN (LastNd);

    List *FDifG = GetFDifG (DB_TYPE_DIF_FUNC, 
                            R_EID2FID (LastDifN->EventId), DifGraph->ThreadId);
    assert (FDifG != NULL && FDifG->Header != NULL);
    
    Node *LastCallNode = (Node *)(FDifG->Header->Data);
    Edge* E = AddDifEdge (DifGraph ,LastCallNode, CurNd);
    SetEdgeType (E, EDGE_CG);

    if (LastCallNode == LastNd)
    {
        SetEdgeType (E, EDGE_CF);
    }

    return;
}

static inline VOID AddInterCfEdge (Graph *DifGraph, Node *LastNd, Node *CurNd)
{
    DifNode* LastDifN = GN_2_DIFN (LastNd);

    List *FDifG = GetFDifG (DB_TYPE_DIF_FUNC, R_EID2FID (LastDifN->EventId), DifGraph->ThreadId);
    assert (FDifG != NULL && FDifG->Header != NULL);
    
    Node *FEntryNode = (Node *)(FDifG->Header->Data);
    Edge* E = AddDifEdge (DifGraph, CurNd, FEntryNode);
    SetEdgeType (E, EDGE_CF);

    LNode *LSecNode = FDifG->Header->Nxt;
    if (LSecNode != NULL)
    {
        assert (LSecNode->Data != NULL);
        E = AddDifEdge (DifGraph, CurNd, (Node *)LSecNode->Data);
        SetEdgeType (E, EDGE_DIF);
    }

    return;
}


static inline VOID AddRetEdge (Graph *DifGraph, Node *LastNd, Node *CurNd)
{
    Edge* E = AddDifEdge (DifGraph, LastNd, CurNd);
    SetEdgeType (E, EDGE_RET);

    return;
}


static inline VOID InsertNode2Graph (Graph *DifGraph, Node *N)
{
    DifNode* DifN = GN_2_DIFN (N);
    DWORD FID = R_EID2FID (DifN->EventId);

    Node *LastGNode = GetLastNode (DifGraph);

    List *FDifG = GetFDifG (DifA.FDifHandle, FID, DifGraph->ThreadId);
    if (FDifG == NULL)
    {
        assert (R_EID2IID (DifN->EventId) == 0);
        FDifG = CreateFDifG (DifA.FDifHandle, FID, DifGraph->ThreadId);

        if (LastGNode != NULL)
        {
            AddCallEdge (DifGraph, LastGNode, N);
        }
    }
    else
    {
        LNode *LN = FDifG->Tail;
        Node *TempN = (Node *)LN->Data;

        // add cf edge
        Edge* E = AddDifEdge (DifGraph, TempN, N);           
        SetEdgeType (E, EDGE_CF);            


        // add dif edge
        UseMap Um = {{0}, 0};
        while (LN != NULL)
        {
            TempN = (Node *)LN->Data;
            if (IsNodeDD (DifN, GN_2_DIFN (TempN), &Um))
            {
                Edge* E = AddDifEdge (DifGraph, TempN, N);
                SetEdgeType (E, EDGE_DIF);
                if (Um.MapNum == DifN->EMsg.Use.NodeNum)
                {
                    break;
                }
            }

            LN = LN->Pre;
        }

        // add ret edge
        DifNode* LastDifN = GN_2_DIFN (LastGNode);
        //ViewEMsg (&LastDifN->EMsg);
        if (R_EID2FID (LastDifN->EventId) != R_EID2FID (DifN->EventId) &&
            R_EID2ETY (DifN->EventId) == EVENT_CALL)
        {
            AddRetEdge (DifGraph, LastGNode, N);
            AddInterCfEdge (DifGraph, LastGNode, N);
        }
    }
  
    ListInsert (FDifG, N);
    AddNode (DifGraph, N);

    return;
}

static inline VOID UpdateThrEvent (ULONG Event, char *Msg)
{
    DbReq Req;
    DbAck Ack;

    EventMsg EMsg = {0};
    DecodeEventMsg (&EMsg, Event, Msg);

    Variable *ThrVal = (Variable *)EMsg.Def.Header->Data;
    char FuncName [FUNC_NAME_LEN]  ={0};
    strncpy (FuncName, ThrVal->Name, sizeof(FuncName));
    DelEventMsg (&EMsg);
    
    Req.dwDataType = DifA.ThrHandle;
    Req.dwKeyLen   = sizeof (FuncName);
    Req.pKeyCtx    = (BYTE*)FuncName;
  

    // Query first
    Ack.dwDataId = 0;
    (VOID)QueryDataByKey(&Req, &Ack);
    if (Ack.dwDataId == 0)
    {
        DWORD Ret = CreateDataByKey (&Req, &Ack);
        assert (Ret == R_SUCCESS);
    }

    ULONG *Evt = (ULONG *)(Ack.pDataAddr);
    *Evt = Event;

    return;  
}

VOID DifEngine (ULONG Event, DWORD ThreadId, char *Msg)
{
    Graph *DifGraph = GetGraph (ThreadId, DifA.NodeHandle, DifA.EdgeHandle);

    if (IsEventExist (DifGraph, Event, ThreadId))
    {
        return;
    }
    
    DEBUG ("[DIF][T:%x]%lx: %s \r\n", ThreadId, Event, Msg);
    Node *N = AddDifNode (DifGraph, Event);
    DifNode* DifN = GN_2_DIFN (N);
    DifN->EventId = Event;

    DecodeEventMsg (&DifN->EMsg, Event, Msg);
    ViewEMsg (&DifN->EMsg);

    InsertNode2Graph (DifGraph, N);
    
    return;
}




