
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

typedef VOID  (*_PLUGIN_ENTRY_) (DWORD SrcHandle, VOID *Plg);
typedef DWORD (*_IS_SINK_) (List *SinkList, Node *DstNode);

typedef struct tag_DynCtx
{
    List LastVisit;
    List Sinks;
}DynCtx;

typedef struct tag_Plugin
{
    CHAR Name[64];
    CHAR Entry[64];
    CHAR Module[64];
    DWORD Active;
    DWORD DataHandle;
    DWORD InitStatus;

    VOID* DbAddr;
    List SinkList;
    _PLUGIN_ENTRY_ PluginEntry;
    _IS_SINK_ IsSink;
}Plugin;


List* InstallPlugins ();
VOID UnInstallPlugins ();
VOID VisitDifg (DWORD SrcHandle, Plugin *Plg);



#endif 
