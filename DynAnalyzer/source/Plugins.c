/***********************************************************
 * Author: Wen Li
 * Date  : 2/1/2020
 * Describe: Plugins.c - plugin install
 * History:
   <1> 2/1/2020 , create
************************************************************/
#include <ctype.h>
#include <Plugins.h>
#include <dlfcn.h>
#include <DifGraph.h>

#define DATA_DIR ("/tmp/difg/")
typedef struct tag_CasesSinks
{
    char PluginName[32];
	char FuncName[128];
    DWORD InstId;
    DifNode *SinkNode;
    DWORD ThreadId;
}CasesSinks;

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
static List PluginList;
static List VisitCache;
static List DetSinks;


List* GetFDifG (DWORD Handle, ULONG FID, DWORD ThreadId);


static inline VOID LoadSinks (List *SinkList, char* CfgName)
{
    char PgnSinks[256];
    snprintf (PgnSinks, sizeof(PgnSinks), "%s%s", DATA_DIR, CfgName);

    
    FILE *Pf = fopen (PgnSinks, "r");
    if (Pf == NULL)
    {
        printf ("@@@@@@@@ PgnSinks[%s] not exist!!!", CfgName);
        return ;
    }

    while (!feof (Pf))
    {
        fscanf (Pf, "%s", PgnSinks);

        unsigned Length = strlen (PgnSinks) + 1;
        char *Sink = (char *)malloc (Length);
        assert (Sink  != NULL);
        strncpy (Sink, PgnSinks, Length);

        ListInsert(SinkList, Sink);
    }
    fclose (Pf);

    return;
}

static DWORD GetFiled (char* Ini, char** Field, char** Value)
{
    DWORD Offset = 0;
    static char F[64];
    static char V[64];

    if (*Ini == 0)
    {
        return 0;
    }

    char *Ctx = Ini;
    if (*Ctx == '[')
    {
        Ctx++;
    }

    memset (F, 0, sizeof(F));    
    memset (V, 0, sizeof(V));

    DWORD Len = 0;
    /* field */
    while (*Ctx != ':' && *Ctx != 0)
    {
        F[Len++] = *Ctx;
        Ctx++;
    }
    F[Len] = 0;
    Ctx++;
    
    /* Value */
    Len = 0;
    while (*Ctx != ',' && *Ctx != ']' && *Ctx != 0)
    {
        V[Len++] = *Ctx;
        Ctx++;
    }
    V[Len] = 0;
    Ctx++;

    *Field = F;
    *Value = V;
    return (DWORD) (Ctx - Ini); 
}

static inline VOID GetPluginCfg (char *Buffer, Plugin *Pgn)
{
    DWORD Offset;
    char *Field, *Value, *BufAddr;    

    /* load configuration */
    BufAddr = Buffer;
    while ((Offset = GetFiled (BufAddr, &Field, &Value)) != 0)
    {  
        if (strstr (Field, "name"))
        {
            snprintf ((char *)Pgn->Name, sizeof (Pgn->Name), "%s", Value);
        }
        if (strstr (Field, "init"))
        {
            snprintf ((char *)Pgn->Init, sizeof (Pgn->Init), "%s", Value);
        }
        else if (strstr (Field, "module"))
        {
            snprintf ((char *)Pgn->Module, sizeof (Pgn->Module), "%s", Value);
        }
        else if (strstr (Field, "sink"))
        {
            LoadSinks (&Pgn->SinkList, Value);
        }
        else if (strstr (Field, "active"))
        {
            Pgn->Active = atoi (Value);
        }
        
        BufAddr += Offset;
    }

    return;
}


static inline VOID GetPluginInit (Plugin *Pgn)
{
    char Buffer[1024];
    snprintf (Buffer, sizeof(Buffer), "%s%s", DATA_DIR, Pgn->Module);
    VOID *PluginSo = dlopen(Buffer, RTLD_LAZY);
    if(dlerror())
    {  
        printf ("Plugin[%s]: %s fail!\r\n", Pgn->Name, Pgn->Module);  
        return;   
    }
            
    Pgn->PluginInit = (_PLUGIN_INIT_)dlsym(PluginSo, (const char *)Pgn->Init);
    if (dlerror())
    {
        printf ("Plugin[%s]: load entry %s[%s] fail!\r\n", Pgn->Name, Pgn->Init, Pgn->Module);  
        return;  
    }
    
    return;
}


List* InstallPlugins ()
{
    FILE *Pf = fopen (PLUGIN_INI, "r");
    if (Pf == NULL)
    {
        printf ("@@@@@@@@ plugins.ini not exist!!!");
        return NULL;
    }

    DWORD DataHandle = DB_TYPE_DIF_PLUGIN_BEGIN;

    char Buffer[1024];
    DWORD LineNo = 1;
    while (!feof (Pf))
    {
        memset (Buffer, 0, sizeof (Buffer));
        fgets (Buffer, sizeof (Buffer), Pf);
        DWORD Len = strlen (Buffer);
        if (Len < 16)
        {
            continue;
        }

        Plugin *Pgn = (Plugin *)malloc (sizeof (Plugin));
        assert (Pgn != NULL);
        
        memset (Pgn, 0, sizeof (Plugin));
        GetPluginCfg (Buffer, Pgn);

        /* load library */
        GetPluginInit (Pgn);

        Pgn->DataHandle = DataHandle++;
        Pgn->DbAddr     = GetDbAddr();
        Pgn->PluginInit (Pgn);
        ListInsert (&PluginList, Pgn);
        DEBUG ("InstallPlugin [%u][%s]%s->%s(%p), Active=%u\r\n", 
                 Pgn->DataHandle, Pgn->Name, Pgn->Module, Pgn->Init, Pgn->PluginInit, Pgn->Active);
    }

    fclose (Pf);

    return &PluginList;
}


static inline VOID DelPlugin (Plugin *Pgn)
{
    if (Pgn != NULL)
    {
        free (Pgn);
    }
    return;
}

VOID UnInstallPlugins ()
{
    ListDel(&PluginList, (DelData)DelPlugin);

    ListDel(&DetSinks, (DelData)free);
}

static inline DWORD SET_VISIT (DWORD Visit, DWORD Type)
{
    Type -= DB_TYPE_DIF_PLUGIN_BEGIN;
    Visit |= (1 << Type);

    return Visit;
}

static inline DWORD GET_VISIT (DWORD Visit, DWORD Type)
{
    Type -= DB_TYPE_DIF_PLUGIN_BEGIN;
    Visit = (Visit >> Type) & 1;

    return Visit;
}

static inline void PrintVar (VOID *Data)
{
    Variable *V = (Variable *)Data;
    printf ("[%c (%s,%lx)] ", V->Type, V->Name, V->Addr);

    return;
}

static void PrintEMsg (unsigned NodeId, unsigned long EventId, EventMsg *EM)
{
    printf ("==========================================================\r\n");
    printf ("[Node%u][%lx]<ViewEMsg> --- [Definition]:", NodeId, EventId);
    ListVisit (&EM->Def, PrintVar);

    printf (" -- [Use]:");
    ListVisit (&EM->Use, PrintVar);
    printf ("\r\n");

    return;    
}



static inline List* GetLastVisit (Node *Source)
{
    if (VisitCache.Header == NULL)
    {
        ListInsert(&VisitCache, Source);
        DEBUG("@@@@@@@ INSERT source: Node%u -> [%u]%p\r\n", Source->Id, VisitCache.NodeNum, VisitCache.Header->Data);
    }

    return &VisitCache;
}


static inline DWORD GetFuncId (DifNode *DN)
{
    ULONG Eid = DN->EventId;

    unsigned Fid  = R_EID2FID(Eid);
    unsigned Lang = R_EID2LANG(Eid);

    return (Fid | (Lang<<28));
}


static inline char* GetFuncName (DifNode *DN, DWORD ThreadId)
{
    DWORD FID = GetFuncId (DN);
     
    List *FDifG = GetFDifG (DB_TYPE_DIF_FUNC, FID, ThreadId);
    assert (FDifG  != NULL);

    Node* N = (Node*)FDifG->Header->Data;
    assert (N != NULL);

    DifNode *difN = GN_2_DIFN (N);
    LNode *DefNode = difN->EMsg.Def.Header;
    while (DefNode != NULL)
    {
        Variable *Val = (Variable *)DefNode->Data;
        if (Val ->Type != VT_FUNCTION)
        {
            continue;
        }

        return Val->Name;
            
        DefNode = DefNode->Nxt;
    }

    return NULL;

}


static inline DWORD InvokePlugins (List* PluginList, DifNode *SrcNode, DifNode *DstNode, DWORD ThreadId)
{
    DWORD SINK = FALSE;
    Plugin *Plg;

    LNode *Header = PluginList->Header;
    while (Header != NULL)
    {
        Plg = (Plugin *)Header->Data;
        assert (Plg != NULL);

        if (Plg->Active)
        {
            DEBUG ("=> ENTRY plugin: %s\r\n", Plg->Name);
            DWORD IsSink = Plg->Detect (Plg, SrcNode, DstNode);
            if (IsSink == TRUE)
            {
                CasesSinks *Cs = (CasesSinks*)malloc (sizeof (CasesSinks));
                assert (Cs != NULL);
                ListInsert(&DetSinks, Cs);
                
                char *FuncName = GetFuncName (DstNode, ThreadId);
                if (FuncName == NULL)
                {
                    FuncName = "Unknown";
                }

                strncpy (Cs->FuncName, FuncName, sizeof (Cs->FuncName));
                strncpy (Cs->PluginName, Plg->Name, sizeof (Cs->PluginName));
                //Cs->LangType = R_EID2LANG(DstNode->EventId);
                Cs->InstId   = R_EID2IID(DstNode->EventId);
                Cs->SinkNode = DstNode;
                Cs->ThreadId = ThreadId;
                
                printf ("\r\n@@@@@@@@@@@@@@@@@@@[%u][%s]Reach sink,  EventId = %u -- <Function:%s,  Inst:%u> \r\n", 
                        Plg->DataHandle, Plg->Name, R_EID2ETY(DstNode->EventId), FuncName, R_EID2IID(DstNode->EventId));
                printf ("\t\t ---->case: %s %s %u \r\n", Plg->Name, FuncName, R_EID2IID(DstNode->EventId));
            }

            SINK |= IsSink;
        }  
        
        Header = Header->Nxt;
    }

    return SINK;
}


static inline VOID ProcSource (Node *Source, List* PluginList, DWORD ThreadId)
{
    List *LastVisit = GetLastVisit (Source);

    while (1)
    {
        DWORD ListChange = FALSE;     
        DWORD NodeNum = LastVisit->NodeNum;
        LNode *Last = LastVisit->Header;
        DEBUG (">>>.......... [%u] Header = %p \r\n", LastVisit->NodeNum, LastVisit->Header);
        Node *RetNode = NULL;
        while (NodeNum > 0)
        {
            Node *N = (Node *)Last->Data;
            DifNode *SrcN = GN_2_DIFN (N);
            DEBUG ("[Node%u][%lx]SRCnode -> FunctionID = %x \r\n", N->Id, SrcN->EventId, GetFuncId (SrcN));
            
            List *OutEdge = &N->OutEdge;
            LNode *LE = OutEdge->Header;
            while (LE != NULL)
            {
                Edge *E = (Edge *)LE->Data;
                DifEdge *DE = GE_2_DIFE (E);
                if (!(DE->EdgeType & EDGE_DIF) && !(DE->EdgeType & EDGE_RET))
                {
                    LE = LE->Nxt;
                    continue;
                }
                Node *DstNode = E->Dst;
                
                if (GET_VISIT(DstNode->VisitBits, DB_TYPE_DIF_PLUGIN_BEGIN))
                {
                    LE = LE->Nxt;
                    continue;   
                }
                else
                {
                    DstNode->VisitBits = SET_VISIT (DstNode->VisitBits, DB_TYPE_DIF_PLUGIN_BEGIN);
                }

                DifNode *DstN = GN_2_DIFN (DstNode);
                if (DE->EdgeType & EDGE_RET)
                {
                    DEBUG ("Reach Return node -> FunctionID = %x \r\n", GetFuncId (DstN));
                    ListInsert(LastVisit, DstNode);
                    LE = LE->Nxt;
                    continue;
                }
   
                //PrintEMsg(DstNode->Id, DstN->EventId, &DstN->EMsg);
                InvokePlugins (PluginList, SrcN, DstN, ThreadId);
                DEBUG ("Go on DSTnode -> EventId = %u (%p) ", R_EID2ETY(DstN->EventId), DstN);
                ListInsert(LastVisit, DstNode);

                ListChange = TRUE;
                LE = LE->Nxt;
                DEBUG ("==============> Set ListChange = TRUE \r\n");
            }

            Last = Last->Nxt;
            NodeNum--;
        }

        if (ListChange == FALSE)
        {
            break;
        }
    }

    return;
}


VOID VisitDifg (DWORD SrcHandle, List* PluginList, DWORD ThreadId)
{
    DbReq Req;
    DbAck Ack;

    /* Visit all sources, and get context of source: (incremental) */
    DWORD SrcNum = QueryDataNum (SrcHandle);
    Req.dwDataType = SrcHandle;
    DEBUG ("@@@@@@@@@@@@@@@@ VisitDifg -> Source num: %u @@@@@@@@@@@@@@@@\r\n", SrcNum);
    while (SrcNum > 0)
    {
        Req.dwDataId = SrcNum;
        DWORD Ret = QueryDataByID(&Req, &Ack);
        assert (Ret != R_FAIL);
        
        Node *Source = *((Node **)(Ack.pDataAddr));
        ProcSource (Source, PluginList, ThreadId);

        SrcNum--;
    }

    DEBUG ("@@@@@@@@@@@@@@@@ VisitDifg -> Source num: %u @@@@@@@@@@@@@@@@\r\n\r\n", SrcNum);
    return;
}


static inline DWORD LoadCases (char *Cases, List* CaseList)
{
    FILE *Pf = fopen (Cases, "r");
    if (Pf == NULL)
    {
        return R_FAIL;
    }

    CasesSinks Csk;
    while (!feof (Pf))
    {
        memset (&Csk, 0, sizeof(Csk));
        fscanf (Pf, "%s %s %u", Csk.PluginName, Csk.FuncName, &Csk.InstId);
        if (Csk.PluginName[0] == 0)
        {
            continue;
        }
    
        printf ("LoadCases -> %s:%s:%u\r\n", Csk.PluginName, Csk.FuncName, Csk.InstId);

        CasesSinks *Cnode = (CasesSinks *)malloc (sizeof (CasesSinks));
        assert (Cnode != NULL);
        memcpy (Cnode, &Csk, sizeof (Csk));
        ListInsert(CaseList, Cnode);
    }
    fclose (Pf);
    
    return R_SUCCESS;
}

VOID CheckCases (char *Cases)
{
    printf ("entry CheckCases ... %s\r\n", Cases);
    if (Cases == NULL)
    {
        return;
    }
    
    List CaseList;
    memset (&CaseList, 0, sizeof (CaseList));
    if (LoadCases (Cases, &CaseList) != R_SUCCESS)
    {
        return;
    }

    
    LNode *Ln = CaseList.Header;
    while (Ln != NULL)
    {
        CasesSinks *Cnode = (CasesSinks *)Ln->Data;

        DWORD OK = FALSE;
        LNode *cLn = DetSinks.Header;
        while (cLn != NULL)
        {
            CasesSinks *Ds = (CasesSinks *)cLn->Data;
            if (strcmp (Cnode->PluginName, Ds->PluginName) == 0 &&
                strcmp (Cnode->FuncName, Ds->FuncName) == 0 &&
                Cnode->InstId == Ds->InstId)
            {
                OK = TRUE;
                break;
            }

            cLn = cLn->Nxt;
        }

        if (OK)
        {
            printf ("@@@@CASE-TEST PASS -> %s-%s:%u\r\n", Cnode->PluginName, Cnode->FuncName, Cnode->InstId);
        }
        else
        {
            printf ("@@@@CASE-TEST FAIL -> %s-%s:%u\r\n", Cnode->PluginName, Cnode->FuncName, Cnode->InstId);
        }
        
        Ln = Ln->Nxt;
    }

    ListDel(&CaseList, (DelData)free);

    return;
}


static inline BOOL IsReachable (DifNode *DifN)
{
    DbReq Req;
    DbAck Ack;

    Req.dwDataType = DB_TYPE_DIF_SOURCES;
    Req.dwKeyLen   = sizeof (DifN->EventId);
    Req.pKeyCtx = (BYTE *)(&DifN->EventId);

    Ack.dwDataId = 0;
    (VOID)QueryDataByKey (&Req, &Ack);
    if (Ack.dwDataId != 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CompEvent (DifNode *Ldata, DifNode *Target)
{
    if (R_EID2FID(Ldata->EventId) == R_EID2FID(Target->EventId))
    {
        return TRUE;        
    }

    return FALSE;
}


static inline VOID AddPathNode (List *Path, DifNode *DifN)
{
    if (ListSearch(Path, (CompData)CompEvent, DifN) ==  TRUE)
    {
        return;
    }

    ListInsert(Path, DifN);
    return;
}

static inline VOID ComputeSsPath (DWORD SrcNum, DifNode *Sink, List *Path)
{
    List LastVisit;
    memset (&LastVisit, 0, sizeof (LastVisit));
    
    ListInsert(&LastVisit, DIFN_2_GN(Sink));

    while (LastVisit.Header != NULL)
    {
        DWORD NodeNum = LastVisit.NodeNum;
        LNode *Last   = LastVisit.Header;
        while (NodeNum > 0)
        {
            Node *N = (Node *)Last->Data;
            DifNode *Dst = GN_2_DIFN (N);
                
            List *InEdge = &N->InEdge;
            LNode *LE = InEdge->Header;
            while (LE != NULL)
            {
                Edge *E = (Edge *)LE->Data;
                DifEdge *DE = GE_2_DIFE (E);
                if (!(DE->EdgeType & EDGE_DIF))
                {
                    LE = LE->Nxt;
                    continue;
                }
                Node *SrcNode = E->Src;

                AddPathNode (Path, GN_2_DIFN(SrcNode));  

                ListInsert(&LastVisit, SrcNode);
                LE = LE->Nxt;
            }

            LNode *NxtNode = Last->Nxt;
            ListRemove(&LastVisit, Last);
            Last = NxtNode;
            NodeNum--;
        }
    }
}


static inline void ViewVar (VOID *Data)
{
    Variable *V = (Variable *)Data;
    printf ("[%c (%s,%lx)] ", V->Type, V->Name, V->Addr);

    return;
}


static inline VOID ShowPath (DWORD No, CasesSinks *Cs, List *Path)
{
    printf ("\t[%-2u][%s] Path: ", No, Cs->PluginName);
    LNode *Ln = Path->Tail;
    while (Ln != NULL)
    {
        DifNode *DstNode = (DifNode *)Ln->Data;

        char *FuncName = GetFuncName (DstNode, Cs->ThreadId);
        if (Ln->Pre != NULL)
        {
            printf (" %s -> ", FuncName);
        }
        else
        {
            printf (" %s: ", FuncName);
        }
        
        Ln = Ln->Pre;
    }

    LNode *Lhr = Path->Header;
    DifNode *DstNode = (DifNode *)Lhr ->Data;
    ListVisit (&DstNode->EMsg.Def, (ProcData)ViewVar);

    printf ("\r\n");
    return;    
}


VOID GenSsPath ()
{    
    DWORD SrcNum = QueryDataNum (DB_TYPE_DIF_SOURCES);
    printf ("@@@@@ GenSsPath -> Souece[%u], Sink[%u]......\r\n", SrcNum, DetSinks.NodeNum);
    if (SrcNum == 0)
    {
        return;
    }

    DWORD No = 1;
    LNode *SsLn = DetSinks.Header;
    while (SsLn != NULL)
    {
        List Path;
        memset (&Path, 0, sizeof (Path));

        CasesSinks *Cs = (CasesSinks *)SsLn->Data;
        AddPathNode (&Path, Cs->SinkNode);
        
        ComputeSsPath (SrcNum, Cs->SinkNode, &Path);
        if (Path.Header != NULL)
        {
            ShowPath(No, Cs, &Path);
        }

        SsLn = SsLn->Nxt;
        ListDel(&Path, NULL);
        No++;
    }

    return;
}

