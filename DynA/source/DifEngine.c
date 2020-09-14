/***********************************************************
 * Author: Wen Li
 * Date  : 9/9/2020
 * Describe: DifEngine.c  
 * History:
   <1> 9/9/2020 , create
************************************************************/
#include "MacroDef.h"
#include "Graph.h"
#include "Event.h"
#include "DifEngine.h"

static Graph DifGraph;


VOID DifEngine (ULONG EventId, char *Msg)
{
    printf ("[DIF]%lx: %s \r\n", EventId, Msg);

    return;
}




