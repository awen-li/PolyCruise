/***********************************************************
 * Author: Wen Li
 * Date  : 7/24/2020
 * Describe: DynTrace.c  
 * History:
   <1> 7/24/2020 , create
************************************************************/
#include <sys/syscall.h>
#include "Queue.h"


#ifdef __cplusplus
extern "C"{
#endif

static int gFullTraceFlag = 0;
static char* SinksDebug = "strcpy strncpy __strncpy_chk strcat memcpy memmove memset sprintf llvm.memcpy.i32 llvm.memcpy.p0i8.p0i8.i32 llvm.memcpy.i64 llvm.memcpy.p0i8.p0i8.i64 llvm.memmove.i32 "\
                          "llvm.memmove.p0i8.p0i8.i32 llvm.memmove.i64 llvm.memmove.p0i8.p0i8.i64 llvm.memset.p0i8.i64 llvm.memset.p0i8.i32 log10 log strlen strncmp memcmp strncasecmp "\
                          "execl execlp execle execv execvp execvpe system popen pow "\
                          "printf __printf_chk fputs fputc putc putchar _IO_putc fprintf vfprintf write fwrite fcntl send syslog log_msg log_oom verbose schedlog fwrite_unlocked fputc_unlocked putc_unlocked putchar_unlocked";

static inline void TraceCheck (char *Msg)
{
    printf("%s\r\n", Msg);
    if (strncmp ("[C][CS]:", Msg, 8) != 0)
    {
        return;
    }

    char *CallFunc = Msg + 8;
    if (strstr (SinksDebug, CallFunc) != NULL)
    {
        printf ("@@@@ Reach sink point: %s \r\n", CallFunc);
    }
}

static int g_TraceSt = TRUE;

void TRC_Start ()
{
    g_TraceSt = TRUE;
}

void TRC_Stop ()
{
    g_TraceSt = FALSE;
}

void TRC_trace0 (ULONG EventId, const char* Msg)
{
    if (gFullTraceFlag)
    {
        TraceCheck ((char *)Msg);
        return;
    }

    if (g_TraceSt == FALSE)
    {
        return;
    }

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

    assert (strlen(Node->QBuf) < BUF_SIZE);
    DEBUG ("[TRC_trace0][T:%u]%lx:[%u]%s\r\n", Node->ThreadId, EventId, (unsigned)strlen(Node->QBuf), Node->QBuf);

    return;   
}


void TRC_trace (ULONG EventId, const char* Format, ...)
{
    va_list ap;

    if (gFullTraceFlag)
    {
        char Msg[1024];
        va_start(ap, Format);
        (void)vsnprintf (Msg, sizeof(Msg), Format, ap);
        va_end(ap);
        TraceCheck (Msg);
        return;
    }

    if (g_TraceSt == FALSE)
    {
        return;
    }

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

    assert (strlen(Node->QBuf) < BUF_SIZE);
    DEBUG ("[TRC_trace][T:%u]%lx:[%u]%s\r\n", Node->ThreadId, EventId, (unsigned)strlen(Node->QBuf), Node->QBuf);

    return;   
}


void TRC_thread (ULONG EventId, char* ThreadEntry, ULONG *ThrId,  char *ThrPara)
{
	va_list ap;

	if (g_TraceSt == FALSE)
    {
        return;
    }
	
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
    char *FullInstm = getenv ("FULL_INSTRUMENTATION");
    if (FullInstm != NULL)
    {
        gFullTraceFlag = 1;
    }
    
    return;
}

void TRC_exit ()
{
    QueueSetExit ();
}

#ifdef __cplusplus
}
#endif


