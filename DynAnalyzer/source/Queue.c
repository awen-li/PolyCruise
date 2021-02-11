/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: Queue.c - FIFO Queue
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <sys/shm.h>
#include "Queue.h"


static Queue *g_Queue = NULL;
static int g_SharedId = 0;

#define Q_2_NODELIST(Q) (QNode *)(Q + 1)

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

static inline void* GetQueueMemory (DWORD IsShared, DWORD Size)
{
    void *MemAddr;
    
    if (IsShared)
    {
        key_t ShareKey = GetKey();
        int SharedId = shmget(ShareKey, Size, 0666);
        if(SharedId == -1)
        {
            SharedId = shmget(ShareKey, Size, 0666|IPC_CREAT);
            assert (SharedId != -1);

            MemAddr = shmat(SharedId, 0, 0);
            assert (MemAddr != (void*)-1);

            memset (MemAddr, 0, Size);
        }
        else
        {
            MemAddr = shmat(SharedId, 0, 0);
            assert (MemAddr != (void*)-1);
        }

        g_SharedId = SharedId;
    }
    else
    {
        MemAddr = malloc (Size);
        assert (MemAddr != NULL);

        memset (MemAddr, 0, Size);
    }
    
    return MemAddr;
}

void InitQueue (unsigned QueueNum)
{
    Queue* Q;

    printf ("@@@@@ start InitQueue\r\n");
    if (g_Queue != NULL)
    {
        printf ("@@@@@ Warning: Repeat comimg into InitQueue: %p-%u\r\n", g_Queue, g_SharedId);
        exit (0);
    }
 
    DWORD Size = sizeof (Queue) + QueueNum * sizeof (QNode);
    Q = (Queue *)GetQueueMemory (TRUE, Size);
    if (Q->NodeNum == 0)
    {
        Q->NodeNum  = QueueNum;
        Q->Exit     = FALSE;

        pthread_rwlockattr_t LockAttr;
        pthread_rwlockattr_setpshared(&LockAttr, PTHREAD_PROCESS_SHARED);
        process_lock_init(&Q->InLock, &LockAttr);
    }

    g_Queue = Q;

    DEBUG ("InitQueue:[%p] [%u] %u\r\n", Q, Q->Hindex, Q->Tindex);
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

    process_lock(&Q->InLock);
    if ((Q->Tindex+1) != Q->Hindex)
    {
        Node = Q_2_NODELIST(Q) + Q->Tindex++;
        Node->Flag = FALSE;

        if (Q->Tindex >= Q->NodeNum)
        {
            Q->Tindex = 0;
        }
    }
    DEBUG ("InQueue: [%p][%u, %u]/%u \r\n", Q, Q->Hindex, Q->Tindex, Q->NodeNum);
    process_unlock(&Q->InLock);
    
    return Node;
}

QNode* FrontQueue ()
{
    Queue* Q = g_Queue;
    if (Q == NULL)
    {
        return NULL;
    }

    QNode* Node = NULL;
    process_lock(&Q->InLock);
    if (Q->Hindex != Q->Tindex)
    {
        Node = (Q_2_NODELIST(Q) + Q->Hindex);
    }
    DEBUG ("FrontQueue: [%p][%u, %u]/%u \r\n", Q, Q->Hindex,Q->Tindex, Q->NodeNum);
    process_unlock(&Q->InLock);
   
    return Node;
}



void OutQueue ()
{
    Queue* Q = g_Queue;
    if (Q == NULL)
    {
        return;
    }

    process_lock(&Q->InLock);
    DEBUG ("OutQueue:[%p] [%u, %u]/%u\r\n", Q, Q->Hindex, Q->Tindex, Q->NodeNum);
    
    Q->Hindex++;
    if (Q->Hindex >= Q->NodeNum)
    {
        Q->Hindex = 0;
    }
    process_unlock(&Q->InLock);

    return;
}


DWORD QueueSize ()
{
    Queue* Q = g_Queue;
    if (Q == NULL)
    {
        return 0;
    }

    process_lock(&Q->InLock);
    DWORD Size = ((Q->Tindex + Q->NodeNum) - Q->Hindex)% Q->NodeNum;
    process_unlock(&Q->InLock);

    return Size;
}

VOID QueueSetExit ()
{
    Queue* Q = g_Queue;
    if (Q == NULL)
    {
        return;
    }

    process_lock(&Q->InLock);
    Q->Exit = TRUE;
    process_unlock(&Q->InLock);
    DEBUG ("QueueSetExit: %u \r\n", Q->Exit);

    return;
}

DWORD QueueGetExit ()
{
    Queue* Q = g_Queue;
    if (Q == NULL)
    {
        return 0;
    }

    DWORD Exit = FALSE;
    process_lock(&Q->InLock);
    Exit = Q->Exit;
    process_unlock(&Q->InLock);

    return Exit;
}

VOID DelQueue ()
{
    if(g_SharedId == 0)
    {
        if (g_Queue != NULL)
        {
            free (g_Queue);
            g_Queue = NULL;
        }
    }
    else
    {
        if(shmdt(g_Queue) == -1)
        {
            printf("shmdt failed\n");
            return;
        }

        if(shmctl(g_SharedId, IPC_RMID, 0) == -1)
        {
            printf("shmctl(IPC_RMID) failed\n");
        }
    }

    return;
}


