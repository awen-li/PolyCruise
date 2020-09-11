
/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Queue.h - FIFO Queue
 * History:
   <1> 7/24/2020 , create
************************************************************/
#ifndef _QUEUE_H_
#define _QUEUE_H_
#include "MacroDef.h"


#define  BUF_SIZE  (128)

typedef struct tag_QNode
{
    ULONG EventId;
    unsigned Flag;
    char QBuf [BUF_SIZE];
}QNode;

typedef struct tag_Queue
{
    QNode *NodeList;
    unsigned NodeNum;
    unsigned Hindex;
    unsigned Tindex;

    mutex_lock_t InLock;  
}Queue;


void InitQueue (unsigned QueueNum);
QNode* InQueue ();
QNode* FrontQueue ();
void OutQueue ();
DWORD IsQueueEmpty ();


#endif 
