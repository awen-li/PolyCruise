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

VOID CheckCases (char *Cases);

void EventProcess (DWORD CrossFlag)
{
    DEBUG ("@@@@@ EventProcess entry..!\r\n");
    
    while (!QueueGetExit() || QueueSize () != 0)
    {        
        QNode *QN = FrontQueue ();
        if (QN == NULL || QN->Flag == FALSE)
        {
            continue;
        }

        if (CrossFlag == FALSE)
        {
            CrossFlag = (DWORD)(R_EID2LANG(QN->EventId) == PYLANG_TY);
        }

        if (CrossFlag)
        {
            DifEngine (QN->EventId, QN->ThreadId, QN->QBuf);
        }
        OutQueue ();
    }


    DEBUG ("Exiting -> ExitFlag: %u, QueueSize: %u\r\n", QueueGetExit(), QueueSize ());
    
    return;
}

static DWORD GetPhyMemUse ()
{
    char FileName[256];
    pid_t pid = getpid();

    sprintf (FileName, "/proc/%u/status", pid);
    FILE *F = fopen (FileName, "r");
    assert (F != NULL);

    char Buf[256] = {0};
    while (!feof(F))
    {
        assert (fgets (Buf, sizeof(Buf), F) != NULL);
        if (strstr(Buf, "VmRSS"))
        {
            break;
        }
    }
    fclose(F);

    DWORD MemSize = 0;
    char ItemName[128];
    sscanf (Buf, "%s %u", ItemName, &MemSize);

    return MemSize;
}

void DynStart (DWORD CrossFlag)
{
    DWORD Ret;
    pthread_t Tid;

    InitQueue (QUEUE_SIZE);

    InitDif ();
   
    EventProcess (CrossFlag);

    return;
}


void DynExit (char *CaseResult)
{
    printf ("@@@@@ Ready to exit, total memory: %u (K)!\r\n", GetPhyMemUse ());

    CheckCases (CaseResult);
    WiteGraph ("DIFG");
    DelQueue ();
    DeInitDif ();

    printf ("@@@@@ DIFA engine exits!\r\n");
    return;
}






