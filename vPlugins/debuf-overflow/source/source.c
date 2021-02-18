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

static inline DWORD IsDD (LNode *DefNode, Variable *Val)
{
    while (DefNode != NULL)
    {
        Variable *DV = (Variable *)DefNode->Data;
        if (DV ->Type != VT_INTEGER)
        {
            DefNode = DefNode->Nxt;
            continue;
        }

        if (strcmp (DV->Name, Val->Name) == 0)
        {
            DEBUG ("[BufOverflow]IsDD = %s \r\n", Val->Name);
            return TRUE;
        }

        DefNode = DefNode->Nxt;
    }

    return FALSE;
}


static inline DWORD Detect (Plugin *Plg, DifNode *SrcNode, DifNode *DstNode)
{
    DEBUG ("[BufOverflow]EventType = %u \r\n", R_EID2ETY(DstNode->EventId));
    if (R_EID2ETY(DstNode->EventId) !=  EVENT_CALL)
    {
        return FALSE;        
    }

    LNode *ValNode = DstNode->EMsg.Def.Header;
    LNode *DefNode = SrcNode->EMsg.Def.Header;
    while (ValNode != NULL)
    {
        Variable *Val = (Variable *)ValNode->Data;
        if (Val->Type != VT_FUNCTION)
        {
            ValNode = ValNode->Nxt;
            continue;
        }
        DEBUG ("[BufOverflow]Dynamic Sink point = %s \r\n", Val->Name);

        List *SinkList  = &Plg->SinkList;
        LNode *SinkNode = SinkList->Header;
        while (SinkNode != NULL)
        {
            char *Function = (char *)SinkNode->Data;
            if (strcmp (Function, Val->Name) == 0)
            {
                break;
            }

            SinkNode = SinkNode->Nxt;
        }

        if (SinkNode != NULL)
        {
            LNode *UseNode = DstNode->EMsg.Use.Header;
            while (UseNode != NULL)
            {
                Val = (Variable *)UseNode->Data;
                if (IsDD (DefNode, Val))
                {
                    return TRUE;
                }
            
                UseNode = UseNode->Nxt;
            }     
        }

        ValNode = ValNode->Nxt;
    }

    return FALSE;
}

void InitBufOverflow (Plugin *Plg)
{
    InitDb(Plg->DbAddr);
    ListVisit(&Plg->SinkList, (ProcData)PrintSink);

    Plg->Detect = (_DETECT_)Detect;
    
    return;
}

