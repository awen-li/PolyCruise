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
#include "Db.h"


static List PluginList;

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
    snprintf (Buffer, sizeof(Buffer), "/tmp/difg/%s", Pgn->Module);
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
        ListInsert (&PluginList, Pgn);
        printf ("InstallPlugin [%u][%s]%s->%s(%p), Active=%u\r\n", 
                 Pgn->DataHandle, Pgn->Name, Pgn->Module, Pgn->Entry, Pgn->PluginEntry, Pgn->Active);
    }

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


