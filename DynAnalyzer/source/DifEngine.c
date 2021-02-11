/***********************************************************
 * Author: Wen Li
 * Date  : 9/9/2020
 * Describe: DifEngine.c  
 * History:
   <1> 9/9/2020 , create
************************************************************/
#include "MacroDef.h"
#include "DifGraph.h"
#include "Plugins.h"

static DifAgent DifA;

static inline VOID InitPlugins ()
{
    DifAgent *DA = &DifA;

    DA->PluginList = InstallPlugins();
    return;
}

static inline VOID DeinitPlugins ()
{
    DifAgent *DA = &DifA;

    UnInstallPlugins();
    DA->PluginList = NULL;
    return;
}


VOID InvokePlugins (Plugin *Plg)
{
    DifAgent *DA = &DifA;
    
    if (Plg->Active == 0)
    {
        return;
    }

    DEBUG("@@@ InvokePlugins: %s \r\n", Plg->Name);
    Plg->PluginEntry (DA->Sources, Plg);
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


static inline VOID AddSource (DWORD Handle, ULONG Event, Node *CurNode)
{
    DbReq Req;
    DbAck Ack;

    Req.dwDataType = Handle;
    Req.dwKeyLen   = sizeof (Event);
    Req.pKeyCtx = (BYTE *)(&Event);

    Ack.dwDataId = 0;
    (VOID)QueryDataByKey (&Req, &Ack);
    if (Ack.dwDataId != 0)
    {
        return;
    }
    
    DWORD Ret = CreateDataByKey (&Req, &Ack);
    assert (Ret == R_SUCCESS);

    *((Node **)(Ack.pDataAddr)) = CurNode;
    printf ("===> Add source [%u:%u]%lx -> %p \r\n", Handle, Ack.dwDataId, Event, CurNode);
    return;
  
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
    //printf ("[%u]LastDifN: %#lx , FDifG = %p/%#x, \r\n", LastNd->Id, LastDifN->EventId, FDifG, Eid2DifgKey (LastDifN->EventId));
    assert (FDifG != NULL && FDifG->Header != NULL);

    Node *FuncNode = (Node *)(FDifG->Header->Data);
    Edge* E = AddDifEdge (DifGraph ,FuncNode, CurNd);
    SetEdgeType (E, EDGE_CG);


    return;
}

static inline Node* GetReferNode (Variable *FmlVal, List *FDifG)
{
    LNode *N = FDifG->Header->Nxt;
    while (N != NULL)
    {
        Node *GNode = (Node *)(N->Data);
        DifNode *DNode = GN_2_DIFN(GNode);
        List *UseList = &DNode->EMsg.Use; 
        LNode *UseNode = UseList->Header;
        while (UseNode != NULL)
        {
            Variable *Uval = (Variable *) (UseNode->Data);
            if (strcmp (Uval->Name, FmlVal->Name) == 0)
            {
                return GNode;
            }
    
            UseNode = UseNode->Nxt;
        }

        N = N->Nxt;
    }

    return NULL;
}


static inline VOID AddInterCfEdge (Graph *DifGraph, Node *LastNd, Node *CurNd)
{
    DifNode* LastDifN = GN_2_DIFN (LastNd);

    List *FDifG = GetFDifG (DB_TYPE_DIF_FUNC, Eid2DifgKey (LastDifN->EventId), DifGraph->ThreadId);
    assert (FDifG != NULL && FDifG->Header != NULL);
    
    Node *FEntryNode = (Node *)(FDifG->Header->Data);
    Edge* E = AddDifEdge (DifGraph, CurNd, FEntryNode);
    SetEdgeType (E, EDGE_CF);

    DifNode* CallNode = GN_2_DIFN (CurNd);
    DWORD EdgeNum = 0;
    
    /* definition */
    List *DefList = &CallNode->EMsg.Def;
    LNode *FmlNode = DefList->Header;
    while (FmlNode != NULL)
    {
        Variable *Val = (Variable *) (FmlNode->Data);
        if (Val->Type != VT_FPARA)
        {
            FmlNode = FmlNode->Nxt;
            continue;            
        }
        
        Node *RefNode = GetReferNode (Val, FDifG);
        if (RefNode != NULL)
        {
            E = AddDifEdge (DifGraph, CurNd, RefNode);
            SetEdgeType (E, EDGE_DIF);
            EdgeNum++;
        }
        
        FmlNode = FmlNode->Nxt;
    }

    /* use */
    List *UseList = &CallNode->EMsg.Use;
    FmlNode = UseList->Header;
    while (FmlNode != NULL)
    {
        Variable *Val = (Variable *) (FmlNode->Data);      
        Node *RefNode = GetReferNode (Val, FDifG);
        if (RefNode != NULL)
        {
            E = AddDifEdge (DifGraph, CurNd, RefNode);
            SetEdgeType (E, EDGE_DIF);
            EdgeNum++;
        }
        
        FmlNode = FmlNode->Nxt;
    }

    if (EdgeNum == 0 && UseList->Header != NULL)
    {
        LNode *CalleeNode = FDifG->Header->Nxt;
        if (CalleeNode != NULL)
        {
            assert (CalleeNode->Data != NULL);
            E = AddDifEdge (DifGraph, CurNd, (Node *)CalleeNode->Data);
            SetEdgeType (E, EDGE_DIF);
        }
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
    
        Req.dwDataType = DifA.AddrMapHandle;
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
    DEBUG ("\t--> Base %lX\r\n", Base);

    Req.dwDataType = DifA.AddrMapHandle;
    Req.dwKeyLen   = sizeof (ULONG);

    ULONG Gep = strtol(Def->Name, NULL, 16);
    Req.pKeyCtx    = (BYTE*)(&Gep);
    
    DWORD Ret = CreateDataByKey (&Req, &Ack);
    assert (Ret == R_SUCCESS);

    ULONG *NewBase = (ULONG *)(Ack.pDataAddr);
    //*NewBase = strtol(Use->Name, NULL, 16);
    *NewBase = Base; /* field insensitive */

    DEBUG ("Maping: %lX  to %lX\r\n", Gep, *NewBase);
    
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
                DEBUG ("===> def share variable\r\n");
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
                DEBUG ("===> use share variable\r\n");
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
    }

    NodePtr  = (Node **)(Ack.pDataAddr);
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
        DEBUG (">>>>> [%u]CreateFDifG: %#x-%p\r\n", N->Id, FID, FDifG);
        
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
        DEBUG ("[DIF]FDifG exists, start to compute dependence \r\n");
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
    DifAgent *DA = &DifA;
    Graph *DifGraph = GetGraph (ThreadId, DA->NodeHandle, DA->EdgeHandle);

    if (IsEventExist (DifGraph, Event, ThreadId))
    {
        DEBUG ("[DIF][T:%lx]: %s exists....\r\n", Event, Msg);
        return;
    }
    
    Node *N = AddDifNode (DifGraph, Event);
    DifNode* DifN = GN_2_DIFN (N);
    DifN->EventId = Event;
    DEBUG ("[DIF][T:%X][%lx]%u: %s \r\n", ThreadId, DifN->EventId, N->Id, Msg);

    if (R_EID2SSD (Event))
    {
        AddSource (DA->Sources, Event, N);
    }

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
    DEBUG ("[DIF]EventType = %u \r\n", EventType);
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
            if (EM->Def.Header && EM->Use.Header)
            {
                UpdataAddrMaping ((Variable*)EM->Def.Header->Data,
                                  (Variable*)EM->Use.Header->Data);
            }
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
    
    /* invoke plugins */
    ListVisit (DA->PluginList, (ProcData)InvokePlugins);

    return;
}


VOID InitDif (List* PluginList)
{
    DWORD Ret;
    DifAgent *DA = &DifA;

    DA->IsFieldSensitive = TRUE;

    DA->NodeHandle    = DB_TYPE_DIF_NODE;
    DA->EdgeHandle    = DB_TYPE_DIF_EDGE;
    DA->FDifHandle    = DB_TYPE_DIF_FUNC;
    DA->ThrHandle     = DB_TYPE_DIF_THR;
    DA->GlvHandle     = DB_TYPE_DIF_GLV;
    DA->ShareHandle   = DB_TYPE_DIF_SHARE;
    DA->AddrMapHandle = DB_TYPE_DIF_ADDRMAPING;
    DA->Sources       = DB_TYPE_DIF_SOURCES;

    InitDb(NULL);
    
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

    Ret = DbCreateTable(DA->AddrMapHandle, sizeof (ULONG), sizeof (ULONG));
    assert (Ret != R_FAIL);

    Ret = DbCreateTable(DA->Sources, sizeof (Node*), sizeof (ULONG));
    assert (Ret != R_FAIL);

    InitPlugins ();

    return;
}


VOID DeInitDif ()
{
    DifAgent *DA = &DifA;

    DWORD GraphNum = GetGraphNum ();

    DWORD GraphId = 1;
    while (GraphId <= GraphNum)
    {
        Graph *G = GetGraphById (GraphId);
        //printf ("[G%u]ThreadId: %x \r\n", GraphId, G->ThreadId);

        ListVisit (&G->NodeList, (ProcData)DelDifNode);
        DelGraph (G);

        GraphId++;
    }

    DelDb ();
    DeinitPlugins();

    return;
}

