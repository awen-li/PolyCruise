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
            return TRUE;
        }

        DefNode = DefNode->Nxt;
    }

    return FALSE;
}


static inline DWORD Detect (Plugin *Plg, DifNode *SrcNode, DifNode *DstNode)
{
    DEBUG ("[DivZero]EventType = %u \r\n", R_EID2ETY(DstNode->EventId));
    if (R_EID2ETY(DstNode->EventId) !=  EVENT_DIV)
    {
        return FALSE;        
    }

    LNode *ValNode = DstNode->EMsg.Use.Header;
    LNode *DefNode = SrcNode->EMsg.Def.Header;
    Variable *FuncVal = NULL;
    while (ValNode != NULL)
    {
        Variable *Val = (Variable *)ValNode->Data;
        if (Val->Type == VT_INTEGER && IsDD (DefNode, Val))
        {
            return TRUE;
        }

        if (Val->Type == VT_FUNCTION)
        {
            FuncVal = Val;
        }

        ValNode = ValNode->Nxt;
    }

    if (FuncVal == NULL)
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

void InitDivZero (Plugin *Plg)
{
    InitDb(Plg->DbAddr);
    ListVisit(&Plg->SinkList, (ProcData)PrintSink);

    Plg->Detect = (_DETECT_)Detect;
    
    return;
}

