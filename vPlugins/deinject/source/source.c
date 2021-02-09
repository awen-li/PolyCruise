/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Plugin: detect injection
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <Plugins.h>
#include <header.h>

VOID PrintSink (char *Data)
{
    DEBUG ("infotrack -- sink: %s\r\n", Data);
    return;
}


static inline DWORD IsSink (List *SinkList, Node *DstNode)
{
    DifNode *DN = GN_2_DIFN (DstNode);
    if (R_EID2ETY(DN->EventId) !=  EVENT_CALL)
    {
        return FALSE;        
    }

    EventMsg *EM   = &DN->EMsg;
    LNode *ValNode = EM->Def.Header;
    Variable *FuncVal;
    while (ValNode != NULL)
    {
        FuncVal = (Variable *)ValNode->Data;
        if (FuncVal->Type == VT_FUNCTION)
        {
            break;
        }

        ValNode = ValNode->Nxt;
    }

    if (ValNode == NULL)
    {
        return FALSE;
    }

    LNode *SinkNode = SinkList->Header;
    while (SinkNode != NULL)
    {
        char *Function = (char *)SinkNode->Data;
        if (strcmp (Function, FuncVal->Name) == 0)
        {
            return TRUE;
        }

        SinkNode = SinkNode->Nxt;
    }

    return FALSE;
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

void DetectInject (DWORD SrcHandle, Plugin *Plg)
{
    DbReq Req;
    DbAck Ack;

    DEBUG ("Entry DetectInject\r\n");
    if (Plg->InitStatus == FALSE)
    {
        InitPluginCtx (Plg);
        ListVisit(&Plg->SinkList, (ProcData)PrintSink);
    }

    /* Visit all sources, and get context of source: (incremental) */
    VisitDifg (SrcHandle, Plg);
    
    return;
}
