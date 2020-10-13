/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: DynTrace.c  
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <sys/syscall.h>
#include "Queue.h"
#include "Event.h"

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

    //printf ("[TRC_trace][T:%x]%lx:%s\r\n", Node->ThreadId, EventId, Node->QBuf);

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

    //printf ("[TRC_thread][T:%X]%lx:%s\r\n", Node->ThreadId, EventId, Node->QBuf);

    return;   
}





