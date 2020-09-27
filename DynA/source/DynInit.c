/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: DynInit.c  
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include "Queue.h"
#include "Graph.h"
#include "DifEngine.h"
#include "Event.h"
#include "GraphViz.h"

void *EventProcess (void* Arg)
{
    while (1)
    {
        QNode *QN = FrontQueue ();
        if (QN == NULL || QN->Flag == FALSE)
        {
            sleep (1);
            continue;
        }
        
        DifEngine (QN->EventId, QN->ThreadId, QN->QBuf);       
        OutQueue ();
    }
    
    return NULL;
}


void TRC_init ()
{
    DWORD Ret;
    pthread_t Tid;
    
    InitQueue (4096);

    InitDif ();
   
    Ret = pthread_create(&Tid, NULL, EventProcess, NULL);
    assert (Ret == 0);

    DEBUG ("TRC_init success!\r\n");
    return;
}


void TRC_exit ()
{
    while (QueueSize ())
    {
        sleep (1);
    }

    sleep (3);
    DEBUG ("TRC_deinit exit!\r\n");

    WiteGraph ("DIFG");
    DelQueue ();
    DeInitDif ();

    return;
}






