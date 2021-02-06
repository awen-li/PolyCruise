/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Plugin: detect CFI
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <Plugins.h>
#include <header.h>

static inline DWORD IsSink (List *SinkList, Node *DstNode)
{
    DifNode *DN = GN_2_DIFN (DstNode);
    if (R_EID2ETY(DN->EventId) ==  EVENT_BR)
    {
        return TRUE;        
    }
    else
    {
        return FALSE;
    }
}

static inline VOID InitPluginCtx (Plugin *Plg)
{
    /* source -> a List of Node (path) */
    DWORD Ret = DbCreateTable(Plg->DataHandle, sizeof (DynCtx), sizeof (Node*));
    assert (Ret != R_FAIL);

    Plg->IsSink = (_IS_SINK_)IsSink;
    Plg->InitStatus = TRUE;
    return;
}

void DetectCfi (DWORD SrcHandle, Plugin *Plg)
{
    DbReq Req;
    DbAck Ack;

    DEBUG ("Entry DetectCfi\r\n");
    if (Plg->InitStatus == FALSE)
    {
        InitPluginCtx (Plg);
    }

    /* Visit all sources, and get context of source: (incremental) */
    VisitDifg (SrcHandle, Plg);
    
    return;
}
