
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
#include <Graph.h>

typedef VOID  (*_PLUGIN_INIT_) (VOID *Plg);
typedef DWORD (*_DETECT_) (VOID *Plg, VOID *SrcNode, VOID *DstNode);

typedef struct tag_Plugin
{
    CHAR Name[64];
    CHAR Init[64];
    CHAR Module[64];
    DWORD Active;
    DWORD DataHandle;

    VOID* DbAddr;
    List SinkList;
    _PLUGIN_INIT_ PluginInit;
    _DETECT_ Detect;

    List DynSinks;
}Plugin;


List* InstallPlugins ();
VOID UnInstallPlugins ();
VOID VisitDifg (DWORD SrcHandle, List* PluginList, DWORD ThreadId);



#endif 
