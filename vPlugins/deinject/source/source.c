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
    printf ("infotrack -- sink: %s\r\n", Data);
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
    while (ValNode != NULL)
    {
        Variable *Val = (Variable *)ValNode->Data;
        if (Val->Type != VT_FUNCTION)
        {
            ValNode = ValNode->Nxt;
            continue;
        }

        LNode *SinkNode = SinkList->Header;
        while (SinkNode != NULL)
        {
            char *Function = (char *)SinkNode->Data;
            if (strcmp (Function, Val->Name) == 0)
            {
                DEBUG ("@@@@@@@@@@@@ Reach sink: ");
                ViewEMsg (&DN->EMsg);
                return TRUE;
            }

            SinkNode = SinkNode->Nxt;
        }

        ValNode = ValNode->Nxt;
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

    printf ("Entry InfoTrack\r\n");
    if (Plg->InitStatus == FALSE)
    {
        InitPluginCtx (Plg);
        ListVisit(&Plg->SinkList, (ProcData)PrintSink);
    }

    /* Visit all sources, and get context of source: (incremental) */
    VisitDifg (SrcHandle, Plg);
    
    return;
}
