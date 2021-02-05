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
        if (strstr (Field, "entry"))
        {
            snprintf ((char *)Pgn->Entry, sizeof (Pgn->Entry), "%s", Value);
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


static inline VOID GetPluginEntry (Plugin *Pgn)
{
    char Buffer[1024];
    snprintf (Buffer, sizeof(Buffer), "%s%s", DATA_DIR, Pgn->Module);
    VOID *PluginSo = dlopen(Buffer, RTLD_LAZY);
    if(dlerror())
    {  
        printf ("Plugin[%s]: %s fail!\r\n", Pgn->Name, Pgn->Module);  
        return;   
    }
            
    Pgn->PluginEntry = (_PLUGIN_ENTRY_)dlsym(PluginSo, (const char *)Pgn->Entry);
    if (dlerror())
    {
        printf ("Plugin[%s]: load entry %s[%s] fail!\r\n", Pgn->Name, Pgn->Entry, Pgn->Module);  
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
        GetPluginEntry (Pgn);

        Pgn->DataHandle = DataHandle++;
        Pgn->InitStatus = FALSE;
        ListInsert (&PluginList, Pgn);
        printf ("InstallPlugin [%u][%s]%s->%s(%p), Active=%u\r\n", 
                 Pgn->DataHandle, Pgn->Name, Pgn->Module, Pgn->Entry, Pgn->PluginEntry, Pgn->Active);
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


static inline DynCtx* GetSrcContext (DWORD Handle, Node *Source)
{
    DbReq Req;
    DbAck Ack;

    Req.dwDataType = Handle;
    Req.dwKeyLen   = sizeof (Node *);
    Req.pKeyCtx    = (BYTE *)(&Source);

    Ack.dwDataId = 0;
    (VOID)QueryDataByKey (&Req, &Ack);
    if (Ack.dwDataId == 0)
    {
        DWORD Ret = CreateDataByKey (&Req, &Ack);
        assert (Ret == R_SUCCESS); 

        DynCtx *Ctx = (DynCtx *)(Ack.pDataAddr);
        List *LastVisit = &Ctx->LastVisit;
        ListInsert(LastVisit, Source);
        DEBUG("@@@@@@@ INSERT source: [%u]%p\r\n", LastVisit->NodeNum, LastVisit->Header->Data);
    }

    return (DynCtx *)(Ack.pDataAddr);
}


static inline DWORD GetFuncId (Node *N)
{
    DifNode *DN = GN_2_DIFN (N);
    ULONG Eid = DN->EventId;

    unsigned Fid  = R_EID2FID(Eid);
    unsigned Lang = R_EID2LANG(Eid);

    return (Fid | (Lang<<28));
}


static inline VOID ProcSource (Plugin *Plg, Node *Source)
{
    DynCtx *Ctx = GetSrcContext (Plg->DataHandle, Source);
    assert (Ctx != NULL);

    List *LastVisit = &Ctx->LastVisit;

    while (1)
    {
        DWORD ListChange = FALSE;     
        DWORD NodeNum = LastVisit->NodeNum;
        LNode *Last = LastVisit->Header;
        DEBUG ("\t\t.......... [%u] Header = %p \r\n", LastVisit->NodeNum, LastVisit->Header);
        Node *RetNode = NULL;
        while (NodeNum > 0)
        {
            Node *N = (Node *)Last->Data;
            DifNode *SrcN = GN_2_DIFN (N);
            DEBUG ("SRCnode -> FunctionID = %x \r\n", GetFuncId (N));
            ViewEMsg (&SrcN->EMsg);

            
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

                if (DE->EdgeType & EDGE_RET)
                {
                    DEBUG ("Reach Return node -> FunctionID = %x \r\n", GetFuncId (DstNode));
                    ListInsert(LastVisit, DstNode);
                    LE = LE->Nxt;
                    continue;
                }
                
                if (GET_VISIT(DstNode->VisitBits, Plg->DataHandle))
                {
                    LE = LE->Nxt;
                    continue;   
                }
                else
                {
                    DstNode->VisitBits = SET_VISIT (DstNode->VisitBits, Plg->DataHandle);
                }
                
                if (Plg->IsSink (&Plg->SinkList, DstNode))
                {
                    ListInsert(&Ctx->Sinks, DstNode);       
                }
                else
                {
                    DifNode *DstN = GN_2_DIFN (DstNode);
                    DEBUG ("DSTnode -> EventId = %u (%p) ", R_EID2ETY(DstN->EventId), DstN);
                    ViewEMsg (&DstN->EMsg);
                    ListInsert(LastVisit, DstNode);
                }

                ListChange = TRUE;
                LE = LE->Nxt;
            }

            Last = Last->Nxt;
            NodeNum--;
        }

        if (ListChange == FALSE)
        {
            DEBUG ("$$$$$$$$$$$$$$ LIST not change, break now......\r\n");
            break;
        }
    }

    return;
}


VOID VisitDifg (DWORD SrcHandle, Plugin *Plg)
{
    DbReq Req;
    DbAck Ack;

    /* Visit all sources, and get context of source: (incremental) */
    DWORD SrcNum = QueryDataNum (SrcHandle);
    Req.dwDataType = SrcHandle;
    while (SrcNum > 0)
    {
        Req.dwDataId = SrcNum;
        DWORD Ret = QueryDataByID(&Req, &Ack);
        assert (Ret != R_FAIL);
        
        Node *Source = *((Node **)(Ack.pDataAddr));
        ProcSource (Plg, Source);

        SrcNum--;
    }
    
    return;
}


