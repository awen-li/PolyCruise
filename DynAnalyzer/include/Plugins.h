
/***********************************************************
 * Author: Wen Li
 * Date  : 2/1/2020
 * Describe: Plugins.h - plugin install
 * History:
   <1> 2/1/2020, create
************************************************************/
#ifndef _PLUGINS_H_
#define _PLUGINS_H_
#include <List.h>

typedef VOID (*_PLUGIN_ENTRY_) (VOID *DifA, VOID* PlgCtx);

typedef struct tag_Plugin
{
    CHAR Name[64];
    CHAR Entry[64];
    CHAR Module[64];
    DWORD Active;
    DWORD DataHandle;
    DWORD InitStatus;

    List SinkList;
    _PLUGIN_ENTRY_ PluginEntry; 
}Plugin;



List* InstallPlugins ();
VOID UnInstallPlugins ();



#endif 
