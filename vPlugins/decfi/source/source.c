/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Plugin: detect CFI
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <Plugins.h>
#include <header.h>

static inline DWORD Detect (Plugin *Plg, DifNode *SrcNode, DifNode *DstNode)
{
    if (R_EID2ETY(DstNode->EventId) ==  EVENT_BR)
    {
        ListInsert(&Plg->DynSinks, DstNode);
        return TRUE;        
    }
    else
    {
        return FALSE;
    }
}

void InitCfi (Plugin *Plg)
{
    InitDb(Plg->DbAddr);

    Plg->Detect = (_DETECT_)Detect;
    
    return;
}
