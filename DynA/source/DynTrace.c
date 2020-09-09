/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: DynTrace.c  
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include "Queue.h"

void TRC_trace (ULONG EventId, const char* Format, ...)
{
	va_list ap;

	
	QNode *Node = InQueue ();
    if (Node == NULL)
    {
        printf ("Queue Full\r\n");
        exit (0);
    }

    Node->EventId = EventId;
	
	va_start(ap, Format);
    (void)vsnprintf (Node->QBuf, sizeof(Node->QBuf), Format, ap);
    va_end(ap);

    //printf ("[%lx]%s \r\n", EventId, Node->QBuf);

    return;   
}




