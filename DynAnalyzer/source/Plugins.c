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

static List PluginList;
static List VisitCache;

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
        printf ("InstallPlugin [%u][%s]%s->%s(%p), Active=%u\r\n", 
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


static inline DWORD GetFuncId (Node *N)
{
    DifNode *DN = GN_2_DIFN (N);
    ULONG Eid = DN->EventId;

    unsigned Fid  = R_EID2FID(Eid);
    unsigned Lang = R_EID2LANG(Eid);

    return (Fid | (Lang<<28));
}

static inline DWORD InvokePlugins (List* PluginList, DifNode *SrcNode, DifNode *DstNode)
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
                printf ("\r\n@@@@@@@@@@@@@@@@@@@[%u][%s]Reach sink,  EventId = %u (%p) \r\n", 
                        Plg->DataHandle, Plg->Name, R_EID2ETY(DstNode->EventId), DstNode);
            }

            SINK |= IsSink;
        }  
        
        Header = Header->Nxt;
    }

    return SINK;
}


static inline VOID ProcSource (Node *Source, List* PluginList)
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
            DEBUG ("[Node%u][%lx]SRCnode -> FunctionID = %x \r\n", N->Id, SrcN->EventId, GetFuncId (N));
            
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

                if (DE->EdgeType & EDGE_RET)
                {
                    DEBUG ("Reach Return node -> FunctionID = %x \r\n", GetFuncId (DstNode));
                    ListInsert(LastVisit, DstNode);
                    LE = LE->Nxt;
                    continue;
                }

                DifNode *DstN = GN_2_DIFN (DstNode);
                PrintEMsg(DstNode->Id, DstN->EventId, &DstN->EMsg);
                if (InvokePlugins (PluginList, SrcN, DstN) == FALSE)
                {
                    DEBUG ("Go on DSTnode -> EventId = %u (%p) ", R_EID2ETY(DstN->EventId), DstN);
                    ListInsert(LastVisit, DstNode);
                }

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


VOID VisitDifg (DWORD SrcHandle, List* PluginList)
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
        ProcSource (Source, PluginList);

        SrcNum--;
    }

    DEBUG ("@@@@@@@@@@@@@@@@ VisitDifg -> Source num: %u @@@@@@@@@@@@@@@@\r\n\r\n", SrcNum);
    return;
}


