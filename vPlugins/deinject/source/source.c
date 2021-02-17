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
    DEBUG ("Inject -- sink: %s\r\n", Data);
    return;
}


static inline DWORD Detect (Plugin *Plg, DifNode *DstNode)
{
    if (R_EID2ETY(DstNode->EventId) !=  EVENT_CALL)
    {
        return FALSE;        
    }

    EventMsg *EM   = &DstNode->EMsg;
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

    List *SinkList  = &Plg->SinkList;
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



void InitInject (Plugin *Plg)
{
    InitDb(Plg->DbAddr);
    ListVisit(&Plg->SinkList, (ProcData)PrintSink);

    Plg->Detect = (_DETECT_)Detect;
    
    return;
}

