/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: DynTrace.c  
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <sys/syscall.h>
#include "Queue.h"

void TRC_trace0 (ULONG EventId, const char* Msg)
{
	QNode *Node = InQueue ();
    if (Node == NULL)
    {
        printf ("Queue Full\r\n");
        exit (0);
    }

    strncpy (Node->QBuf, Msg, sizeof(Node->QBuf));
    Node->ThreadId = pthread_self ();
    Node->EventId  = EventId;
    Node->Flag     = TRUE;

    DEBUG ("[TRC_trace0][T:%u]%lx:[%u]%s\r\n", Node->ThreadId, EventId, (unsigned)strlen(Node->QBuf), Node->QBuf);

    return;   
}


void TRC_trace (ULONG EventId, const char* Format, ...)
{
	va_list ap;
	
	QNode *Node = InQueue ();
    if (Node == NULL)
    {
        printf ("Queue Full\r\n");
        exit (0);
    }

    va_start(ap, Format);
    (void)vsnprintf (Node->QBuf, sizeof(Node->QBuf), Format, ap);
    va_end(ap);

    //Node->ThreadId = syscall(SYS_gettid);
    Node->ThreadId = pthread_self ();
    Node->EventId  = EventId;
    Node->Flag     = TRUE;

    DEBUG ("[TRC_trace][T:%u]%lx:[%u]%s\r\n", Node->ThreadId, EventId, (unsigned)strlen(Node->QBuf), Node->QBuf);

    return;   
}


void TRC_thread (ULONG EventId, char* ThreadEntry, ULONG *ThrId,  char *ThrPara)
{
	va_list ap;
	
	QNode *Node = InQueue ();
    if (Node == NULL)
    {
        printf ("Queue Full\r\n");
        exit (0);
    }

    (void)snprintf (Node->QBuf, sizeof(Node->QBuf), "{%X:%s:%lX}", *((DWORD*)ThrId), ThreadEntry, (ULONG)ThrPara);
    Node->ThreadId = pthread_self ();
    Node->EventId  = EventId;
    Node->Flag     = TRUE;

    DEBUG ("[TRC_thread][T:%X]%lx:%s\r\n", Node->ThreadId, EventId, Node->QBuf);

    return;   
}


void TRC_init ()
{
    return;
}

void TRC_exit ()
{
    QueueSetExit ();
}



