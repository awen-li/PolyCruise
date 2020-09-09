/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: DfiEngine.c  
 * History:
   <1> 9/9/2020 , create
************************************************************/
#include "MacroDef.h"
#include "DfiEngine.h"


VOID DfiEngine (ULONG EventId, char *Msg)
{
    printf ("[DFI]%lx: %s \r\n", EventId, Msg);

    return;
}




