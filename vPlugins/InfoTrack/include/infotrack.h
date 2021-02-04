
/***********************************************************
 * Author: Wen Li
 * Date  : 2/1/2021
 * Describe: infotrack
 * History:
   <1> 2/1/2021 , create
************************************************************/
#ifndef _INFOTACK_H_
#define _INFOTACK_H_
#include "MacroDef.h"
#include "DifGraph.h"
#include <Db.h>


typedef struct tag_ItCtx
{
    List LastVisit;
    List Sinks;
}ItCtx;


#endif 
