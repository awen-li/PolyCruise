/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Queue.c - FIFO Queue
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <Plugins.h>
#include "infotrack.h"

void InfoTrack (DifAgent *DifA, Plugin *PlgCtx)
{
    printf ("entry plugin: [%u]%s\r\n", PlgCtx->DataHandle, PlgCtx->Name);

    
    return;
}