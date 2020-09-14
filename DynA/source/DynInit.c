/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: DynInit.c  
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include "Queue.h"
#include "Db.h"
#include "Graph.h"
#include "DifEngine.h"
#include "Event.h"

void *FilterThread (void* Arg)
{
    DWORD Ret;
    DbReq Req;
    DbAck Ack;

    Req.dwDataType = DB_TYPE_EVENT;
    Req.dwKeyLen   = sizeof (ULONG);

    while (1)
    {
        QNode *QN = FrontQueue ();
        if (QN == NULL || QN->Flag == FALSE)
        {
            sleep (1);
            continue;
        }

        //DEBUG ("Queue: [%lx]%s\r\n", Node->EventId, Node->QBuf);
        if (R_EID2IID (QN->EventId) != 0)
        {
            Req.pKeyCtx  = (BYTE*)(&QN->EventId);
            Ack.dwDataId = 0;
            
            Ret = QueryDataByKey(&Req, &Ack);
            if (Ack.dwDataId != 0)
            {
                OutQueue ();
                continue;
            }

            (VOID)CreateDataByKey (&Req, &Ack);
        }

        DifEngine (QN->EventId, QN->QBuf);
        OutQueue ();
    }
    
    return NULL;
}


void TRC_init ()
{
    pthread_t Tid;
    
    InitQueue (4096);

    DWORD Ret;
    Ret = DbCreateTable(DB_TYPE_EVENT, sizeof (Node), sizeof (ULONG), 128*1024);
    assert (Ret != R_FAIL);
   
    Ret = pthread_create(&Tid, NULL, FilterThread, NULL);
    assert (Ret == 0);

    DEBUG ("TRC_init success!\r\n");
    return;
}


void TRC_exit ()
{
    while (!IsQueueEmpty ())
    {
        sleep (1);
    }

    sleep (1);
    DEBUG ("TRC_deinit exit!\r\n");

    DelQueue ();
    DelDb ();

    return;
}






