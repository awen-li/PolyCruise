/***********************************************************
 * Author: Wen Li
 * Date  : 09/30/2021
 * Describe: Plugin: detect information leakage
 * History:
   <1> 09/30/2021 , create
************************************************************/
#include <Plugins.h>
#include <header.h>

typedef struct
{
    unsigned InstanceCall;
}TypeErrSt;

VOID PrintSink (char *Data)
{
    DEBUG ("Leak -- sink: %s\r\n", Data);
    return;
}

static inline unsigned PathCallValid (char *Call)
{
    if (strcmp (Call, "isinstance") == 0)
    {
        return TRUE;
    }

    return FALSE;
}

static inline void SetInstCall (Plugin *Plg, unsigned Falg)
{
    TypeErrSt *Ts = (TypeErrSt *)Plg->PgData;
    Ts->InstanceCall = Falg;

    printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ SetInstCall !!!!!! \r\n");

    return;
}

static inline DWORD Detect (Plugin *Plg, DifNode *SrcNode, DifNode *DstNode)
{
    if (R_EID2ETY(DstNode->EventId) !=  EVENT_CALL)
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

        if (PathCallValid (Val->Name) == TRUE)
        {
            SetInstCall (Plg, TRUE);
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

void InitTypeErr (Plugin *Plg)
{
    assert (sizeof (TypeErrSt) <= sizeof (Plg->PgData));
    
    InitDb(Plg->DbAddr);
    ListVisit(&Plg->SinkList, (ProcData)PrintSink);

    Plg->Detect = (_DETECT_)Detect;

    TypeErrSt *Ts = (TypeErrSt *)Plg->PgData;
    Ts->InstanceCall = FALSE;
    
    return;
}

