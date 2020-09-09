/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Queue.c - FIFO Queue
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include "Queue.h"


static Queue g_Queue;

void InitQueue (unsigned QueueNum)
{
    Queue* Q = &g_Queue;
    
    if (Q->NodeList == NULL)
    {
        Q->NodeList = (QNode*) malloc (QueueNum * sizeof (QNode));
        if (Q->NodeList == NULL)
        {
            printf ("Q->NodeList malloc fail!\r\n");
            exit (0);
        }

        memset (Q->NodeList, 0, QueueNum * sizeof (QNode));
        Q->Hindex  = 0;
        Q->Tindex  = 0;
        Q->NodeNum = QueueNum;
    }

    return;
}


QNode* InQueue ()
{
    Queue* Q = &g_Queue;
    
    if ((Q->Tindex+1)%Q->NodeNum == Q->Hindex)
    {
        return NULL;
    }

    return (Q->NodeList + Q->Tindex++);
}


QNode* OutQueue ()
{
    Queue* Q = &g_Queue;
    
    if ((Q->Hindex+1)%Q->NodeNum == Q->Tindex)
    {
        return NULL;
    }

    return (Q->NodeList + Q->Hindex++);
}


DWORD IsQueueEmpty ()
{
    Queue* Q = &g_Queue;
    
    return (DWORD)((Q->Hindex+1)%Q->NodeNum == Q->Tindex);
}


