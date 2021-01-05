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

    DA->IsFieldSensitive = TRUE;

    DA->NodeHandle = DB_TYPE_DIF_NODE;
    DA->EdgeHandle = DB_TYPE_DIF_EDGE;
    DA->FDifHandle = DB_TYPE_DIF_FUNC;
    DA->ThrHandle  = DB_TYPE_DIF_THR;
    DA->GlvHandle  = DB_TYPE_DIF_GLV;
    DA->ShareHandle= DB_TYPE_DIF_SHARE;
    DA->AMHandle   = DB_TYPE_DIF_ADDRMAPING;
    
    Ret = DbCreateTable(DA->NodeHandle, sizeof (Node)+sizeof (DifNode), sizeof (EventKey));
    assert (Ret != R_FAIL);

    Ret = DbCreateTable(DA->EdgeHandle, sizeof (Edge)+sizeof (DifEdge), sizeof (Edge));
    assert (Ret != R_FAIL);
    
    Ret = DbCreateTable(DA->FDifHandle, sizeof(List), sizeof (EventKey));
    assert (Ret != R_FAIL);
    
    Ret = DbCreateTable(DA->ThrHandle, sizeof (Node*), sizeof (DWORD));
    assert (Ret != R_FAIL);

    Ret = DbCreateTable(DA->GlvHandle, sizeof (Node*), sizeof (ULONG));
    assert (Ret != R_FAIL);

    Ret = DbCreateTable(DA->ShareHandle, sizeof (DWORD), sizeof (ULONG));
    assert (Ret != R_FAIL);

    Ret = DbCreateTable(DA->AMHandle, sizeof (ULONG), sizeof (ULONG));
    assert (Ret != R_FAIL);

    return;
}

static inline DWORD IsFieldSensitive ()
{
    DifAgent *DA = &DifA;

    return DA->IsFieldSensitive;
}

static inline unsigned Eid2DifgKey (unsigned long Eid)
{
    unsigned Fid  = R_EID2FID(Eid);
    unsigned Lang = R_EID2LANG(Eid);

    return (Fid | (Lang<<28));
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

        DEBUG ("\t===>Prenode definition: %s \r\n", DV->Name);

        UseNo = 0;
        LNode *Use = CurNode->EMsg.Use.Header;
        while (Use != NULL)
        {
            Variable *UV = (Variable *)Use->Data;
            assert (UV != NULL);

            DEBUG ("\t===>Curnode use: %s \r\n", UV->Name);
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
    DifNode* CurDifN  = GN_2_DIFN (CurNd);

    List *FDifG = GetFDifG (DB_TYPE_DIF_FUNC, 
                            Eid2DifgKey (LastDifN->EventId), DifGraph->ThreadId);
    assert (FDifG != NULL && FDifG->Header != NULL);

    if (R_EID2LANG (CurDifN->EventId) == R_EID2LANG (LastDifN->EventId))
    {
        Node *LastCallNode = (Node *)(FDifG->Header->Data);
        Edge* E = AddDifEdge (DifGraph ,LastCallNode, CurNd);
        SetEdgeType (E, EDGE_CG);

        DifNode* LastCallDifN = GN_2_DIFN (LastCallNode);
        if (LastCallNode == LastNd && 
            R_EID2ETY (LastCallDifN->EventId) == EVENT_CALL)
        {
            SetEdgeType (E, EDGE_CF);
        }
    }
    else
    {  
        Node *LastCallNode = (Node *)(FDifG->Tail->Data);
        Edge* E = AddDifEdge (DifGraph ,LastCallNode, CurNd);
        SetEdgeType (E, EDGE_CG);

        if (LastCallNode == LastNd)
        {
            SetEdgeType (E, EDGE_CF);
        }       
    }

    return;
}

static inline VOID AddInterCfEdge (Graph *DifGraph, Node *LastNd, Node *CurNd)
{
    DifNode* LastDifN = GN_2_DIFN (LastNd);

    List *FDifG = GetFDifG (DB_TYPE_DIF_FUNC, Eid2DifgKey (LastDifN->EventId), DifGraph->ThreadId);
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


static inline ULONG GetBaseAddr (Variable *Use)
{
    ULONG Gep = strtol(Use->Name, NULL, 16);
    
    if (IsFieldSensitive ())
    {
        return Gep;
    }
    else
    {
        DbReq Req;
        DbAck Ack;
    
        Req.dwDataType = DifA.AMHandle;
        Req.dwKeyLen   = sizeof (ULONG);        
        Req.pKeyCtx    = (BYTE*)(&Gep);      
        DWORD Ret = QueryDataByKey (&Req, &Ack);
        if (Ret != R_SUCCESS)
        {
            return Gep;
        }
  
        return *(ULONG *)(Ack.pDataAddr); 
    }   
}


static inline VOID UpdataAddrMaping (Variable *Def, Variable *Use)
{
    DbReq Req;
    DbAck Ack;

    ULONG Base = GetBaseAddr (Use);
    printf ("\t--> Base %lX\r\n", Base);

    Req.dwDataType = DifA.AMHandle;
    Req.dwKeyLen   = sizeof (ULONG);

    ULONG Gep = strtol(Def->Name, NULL, 16);
    Req.pKeyCtx    = (BYTE*)(&Gep);
    
    DWORD Ret = CreateDataByKey (&Req, &Ack);
    assert (Ret == R_SUCCESS);

    ULONG *NewBase = (ULONG *)(Ack.pDataAddr);
    //*NewBase = strtol(Use->Name, NULL, 16);
    *NewBase = Base; /* field insensitive */

    printf ("Maping: %lX  to %lX\r\n", Gep, *NewBase);
    
    return;
}



static inline VOID UpdateThrEvent (Node *ThrcNode, Variable *VThrId)
{
    DbReq Req;
    DbAck Ack;

    Req.dwDataType = DifA.ThrHandle;
    Req.dwKeyLen   = sizeof (DWORD);

    DWORD ThreadId = strtol(VThrId->Name, NULL, 16);
    Req.pKeyCtx    = (BYTE*)(&ThreadId);
    
    DWORD Ret = CreateDataByKey (&Req, &Ack);
    assert (Ret == R_SUCCESS);

    Node **NodePtr = (Node **)(Ack.pDataAddr);
    *NodePtr = ThrcNode;

    DEBUG ("UpdateThrEvent: %x \r\n", ThreadId);

    return;  
}

static inline Node* GetThreadEntry (DWORD ThrId)
{
    DbReq Req;
    DbAck Ack;

    Req.dwDataType = DifA.ThrHandle;
    Req.dwKeyLen   = sizeof (DWORD);
    Req.pKeyCtx    = (BYTE*)(&ThrId);

    DWORD Ret = QueryDataByKey (&Req, &Ack);
    if (Ret != R_SUCCESS)
    {
        return NULL;
    }

    Node **NodePtr = (Node **)(Ack.pDataAddr);
    return *NodePtr; 
}


static inline VOID AddThreadEdge (Graph *DifGraph, Node *ThrcNd, Node *CurNd)
{
    Edge* E = AddDifEdge (DifGraph, ThrcNd, CurNd);
    SetEdgeType (E, EDGE_THRC);

    return;
}

static inline VOID UpdateShareVariable (Variable *Share)
{
    DbReq Req;
    DbAck Ack;
    ULONG ShareAddr;

    ShareAddr = GetBaseAddr (Share);
    
    Req.dwDataType = DifA.ShareHandle;
    Req.dwKeyLen   = sizeof (ULONG);   
    Req.pKeyCtx    = (BYTE*)(&ShareAddr);

    /* query first */
    DWORD Ret = QueryDataByKey (&Req, &Ack);
    if (Ret != R_SUCCESS)
    {
        Ret = CreateDataByKey (&Req, &Ack);
        assert ((Ret == R_SUCCESS));
    }

    return;
}


static inline DWORD IsShareVariable (Variable *Var)
{
    DbReq Req;
    DbAck Ack;
    ULONG ShareAddr;

    ShareAddr = GetBaseAddr (Var);

    Req.dwDataType = DifA.ShareHandle;
    Req.dwKeyLen   = sizeof (ULONG);
    Req.pKeyCtx    = (BYTE*)(&ShareAddr);

    /* query first */
    DWORD Ret = QueryDataByKey (&Req, &Ack);
    if (Ret != R_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
}


static inline Variable* IsDefGlv (EventMsg *EM)
{
    LNode *DefHdr = EM->Def.Header;

    while (DefHdr != NULL)
    {
        Variable *V = (Variable *) (DefHdr->Data);
        if (V->Type == VT_GLOBAL)
        {
            return V;
        }
        else if (V->Type == VT_POINTER)
        {
            if (IsShareVariable (V))
            {
                printf ("===> def share variable\r\n");
                return V;
            }
        }

        DefHdr = DefHdr->Nxt;
    }

    return NULL;
}

static inline Variable* IsUseGlv (EventMsg *EM)
{
    LNode *UseHdr = EM->Use.Header;

    while (UseHdr != NULL)
    {
        Variable *V = (Variable *) (UseHdr->Data);
        if (V->Type == VT_GLOBAL)
        {
            return V;
        }
        else if (V->Type == VT_POINTER)
        {
            if (IsShareVariable (V))
            {
                printf ("===> use share variable\r\n");
                return V;
            }
        }

        UseHdr = UseHdr->Nxt;
    }

    return NULL;
}

static inline VOID UpdateGlv (Node *GlvNode, Variable *Glv)
{
    DbReq Req;
    DbAck Ack;
    Node **NodePtr;

    ULONG GlvAddr  = GetBaseAddr (Glv);

    Req.dwDataType = DifA.GlvHandle;
    Req.dwKeyLen   = sizeof (ULONG);
    Req.pKeyCtx    = (BYTE*)(&GlvAddr);

    /* query first */
    DWORD Ret = QueryDataByKey (&Req, &Ack);
    if (Ret != R_SUCCESS)
    {
        Ret = CreateDataByKey (&Req, &Ack);
        assert ((Ret == R_SUCCESS));

        NodePtr  = (Node **)(Ack.pDataAddr);
    }

    *NodePtr = GlvNode;
    return;
}

static inline Node* IsGlvAccess (DifNode* DifN)
{
    Variable *Glv = IsUseGlv (&DifN->EMsg);
    if (Glv == NULL)
    {
        return NULL;
    }
    
    DbReq Req;
    DbAck Ack;

    ULONG GlvAddr  = GetBaseAddr (Glv);

    Req.dwDataType = DifA.GlvHandle;
    Req.dwKeyLen   = sizeof (ULONG);
    Req.pKeyCtx    = (BYTE*)(&GlvAddr);

    DWORD Ret = QueryDataByKey (&Req, &Ack);
    if (Ret != R_SUCCESS)
    {
        return NULL;
    }

    Node **NodePtr = (Node **)(Ack.pDataAddr);
    return *NodePtr; 
}


static inline VOID AddGlvAccessEdge (Graph *DifGraph, Node *CurNd)
{
    Node *GlvcNd = IsGlvAccess (GN_2_DIFN (CurNd));
    if (GlvcNd == NULL || (GlvcNd == CurNd))
    {
        return;
    }
    
    Edge* E = AddDifEdge (DifGraph, GlvcNd, CurNd);
    SetEdgeType (E, EDGE_DIF);

    return;
}

static inline VOID AddSharePropagateEdge (Graph *DifGraph, Node *CurNd)
{
    Node *ThrcNode = DifGraph->ThrcNode;
    if (ThrcNode == NULL)
    {
        return;
    }
    DifNode* DifThrcN = GN_2_DIFN (ThrcNode);
    Variable *ShareVal = (Variable *)DifThrcN->EMsg.Use.Tail->Data;

    DifNode* DifN = GN_2_DIFN (CurNd);
    LNode *UseNode = DifN->EMsg.Use.Header;
    while (UseNode != NULL)
    {
        Variable *Val = (Variable *)UseNode->Data;
        if (strcmp(Val->Name, ShareVal->Name) == 0)
        {
            Edge* E = AddDifEdge (DifGraph, ThrcNode, CurNd);
            SetEdgeType (E, EDGE_TDIF);
        }

        UseNode = UseNode->Nxt;
    }

    return;
}



static inline VOID InsertNode2Graph (Graph *DifGraph, Node *N)
{
    DifNode* DifN = GN_2_DIFN (N);
    DWORD FID = Eid2DifgKey (DifN->EventId);

    Node *LastGNode = GetLastNode (DifGraph);

    List *FDifG = GetFDifG (DifA.FDifHandle, FID, DifGraph->ThreadId);
    if (FDifG == NULL)
    {
        FDifG = CreateFDifG (DifA.FDifHandle, FID, DifGraph->ThreadId);

        if (LastGNode != NULL)
        {
            AddCallEdge (DifGraph, LastGNode, N);
        }
        else
        {
            /* the entry of sub-graph */
            Node *ThrcNode = GetThreadEntry (DifGraph->ThreadId);
            if (ThrcNode != NULL)
            {
                AddThreadEdge (DifGraph, ThrcNode, N);
                DifGraph->ThrcNode = ThrcNode;
            }
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

        /* dependence not found, try dependence cross-threads */
        if (LN == NULL)
        {
            AddSharePropagateEdge (DifGraph, N);
        }

        // add ret edge
        DifNode* LastDifN = GN_2_DIFN (LastGNode);
        //ViewEMsg (&LastDifN->EMsg);
        if (Eid2DifgKey (LastDifN->EventId) != Eid2DifgKey (DifN->EventId) &&
            R_EID2ETY (DifN->EventId) == EVENT_CALL)
        {
            AddRetEdge (DifGraph, LastGNode, N);
            AddInterCfEdge (DifGraph, LastGNode, N);
        }

        AddGlvAccessEdge (DifGraph, N);
    }
  
    ListInsert (FDifG, N);
    AddNode (DifGraph, N);

    return;
}


VOID DifEngine (ULONG Event, DWORD ThreadId, char *Msg)
{
    Graph *DifGraph = GetGraph (ThreadId, DifA.NodeHandle, DifA.EdgeHandle);

    if (IsEventExist (DifGraph, Event, ThreadId))
    {
        return;
    }
    
    DEBUG ("[DIF][%lx][T:%X]%lx: %s \r\n", Event, ThreadId, Event, Msg);
    Node *N = AddDifNode (DifGraph, Event);
    DifNode* DifN = GN_2_DIFN (N);
    DifN->EventId = Event;

    DecodeEventMsg (&DifN->EMsg, Event, Msg);
    EventMsg *EM = &DifN->EMsg; 
    //ViewEMsg (EM);

    /* update Glv database */
    Variable *Glv = IsDefGlv (EM);
    if (Glv != NULL)
    {
        UpdateGlv (N, Glv);
    }

    DWORD EventType = R_EID2ETY (Event);
    switch (EventType)
    {
        case EVENT_THRC:
        {
            /* control flow between threads */
            Variable *VThrId  = (Variable*)EM->Def.Header->Data;
            UpdateThrEvent (N, VThrId);

            Variable *VThrPara  = (Variable*)EM->Use.Header->Data;
            UpdateShareVariable (VThrPara);
            break;
        }
        case EVENT_GEP:
        {
            /* address mapping -> base address */
            UpdataAddrMaping ((Variable*)EM->Def.Header->Data,
                              (Variable*)EM->Use.Header->Data);
            break;
        }
        case EVENT_STORE:
        case EVENT_CALL:
        {
            /* definition check for pointer */
            break;
        }
        default:
        {
            break;            
        }
    }

    InsertNode2Graph (DifGraph, N);
    
    
    return;
}




