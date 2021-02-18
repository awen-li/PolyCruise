/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Plugin: detect buffer overflow
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <Plugins.h>
#include <header.h>

VOID PrintSink (char *Data)
{
    DEBUG ("Overflow -- sink: %s\r\n", Data);
    return;
}


static inline DWORD Detect (Plugin *Plg, DifNode *SrcNode, DifNode *DstNode)
{
    DWORD EventType = R_EID2ETY(DstNode->EventId);
    if (EventType !=  EVENT_ADD &&
        EventType !=  EVENT_MUL)
    {
        return FALSE;        
    }

    EventMsg *EM   = &DstNode->EMsg;
    LNode *ValNode = EM->Def.Header;
    while (ValNode != NULL)
    {
        Variable *Val = (Variable *)ValNode->Data;
        if (Val->Type != VT_FUNCTION)
        {
            ValNode = ValNode->Nxt;
            continue;
        }

        List *SinkList  = &Plg->SinkList;
        LNode *SinkNode = SinkList->Header;
        while (SinkNode != NULL)
        {
            char *Function = (char *)SinkNode->Data;
            if (strcmp (Function, Val->Name) == 0)
            {
                return TRUE;
            }

            SinkNode = SinkNode->Nxt;
        }

        ValNode = ValNode->Nxt;
    }

    return FALSE;
}

void InitIntOverflow (Plugin *Plg)
{
    InitDb(Plg->DbAddr);
    ListVisit(&Plg->SinkList, (ProcData)PrintSink);

    Plg->Detect = (_DETECT_)Detect;
    
    return;
}

