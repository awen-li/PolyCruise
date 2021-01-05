/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Queue.c - FIFO Queue
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <sys/shm.h>
#include "Queue.h"

#define SHARE_KEY (0xC3B3C5D0)

static Queue *g_Queue = NULL;
static int g_SharedId = 0;

static inline key_t GetKey ()
{
    char *ShareKey = getenv("LDI_SHARE_KEY");
    if (ShareKey == NULL)
    {
        return SHARE_KEY;
    }
    else
    {
        return (key_t)strtol(ShareKey, NULL, 16);       
    }  
}

void InitQueue (unsigned QueueNum)
{
    VOID *SharedMemroy = NULL;
	int SharedId;
    Queue* Q = NULL;

    key_t ShareKey = GetKey();
    unsigned Size = sizeof (Queue) + QueueNum * sizeof (QNode);
    SharedId = shmget(ShareKey, Size, 0666);
    if(SharedId == -1)
    {
        SharedId = shmget(ShareKey, Size, 0666|IPC_CREAT);
        assert (SharedId != -1);
    }

    SharedMemroy = shmat(SharedId, 0, 0);
    assert (SharedMemroy != (void*)-1);    

    Q = (Queue *)SharedMemroy;
    printf ("SharedMemroy: %p\r\n", SharedMemroy);
    Q->NodeList = (QNode *)(Q+1);
    Q->Hindex  = 0;
    Q->Tindex  = 0;
    Q->NodeNum = QueueNum;
    mutex_lock_init(&Q->InLock);

    g_Queue = Q;
    g_SharedId = SharedId;

    return;
}


QNode* InQueue ()
{
    if (g_Queue == NULL)
    {
        InitQueue (4096);
    }
    
    Queue* Q = g_Queue;
    QNode* Node = NULL;

    mutex_lock(&Q->InLock);
    if ((Q->Tindex+1)%Q->NodeNum != Q->Hindex)
    {
        Node = Q->NodeList + Q->Tindex++;
        Node->Flag = FALSE;

        if (Q->Tindex >= Q->NodeNum)
        {
            Q->Tindex = 0;
        }
    }
    mutex_unlock(&Q->InLock);
    
    return Node;
}

QNode* FrontQueue ()
{
    Queue* Q = g_Queue;
    
    if (Q->Hindex == Q->Tindex)
    {
        return NULL;
    }

    return (Q->NodeList + Q->Hindex);
}



void OutQueue ()
{
    Queue* Q = g_Queue;
    
    Q->Hindex++;
    if (Q->Hindex >= Q->NodeNum)
    {
        Q->Hindex = 0;
    }

    return;
}


DWORD QueueSize ()
{
    Queue* Q = g_Queue;

    return (Q->Tindex - Q->Hindex);
}

VOID DelQueue ()
{
    if(g_SharedId == 0)
    {
        return;
    }

    if(shmdt(g_Queue) == -1)
	{
		fprintf(stderr, "shmdt failed\n");
		return;
	}

	if(shmctl(g_SharedId, IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
	}

    return;
}


