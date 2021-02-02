
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


typedef VOID (*_PLUGIN_ENTRY_) (VOID *Data);

typedef struct tag_Plugin
{
    char Name[64];
    char Entry[64];
    char Module[64];
    DWORD Active;
    _PLUGIN_ENTRY_ PluginEntry; 
}Plugin;

List* InstallPlugins ();
VOID UnInstallPlugins ();



#endif 
