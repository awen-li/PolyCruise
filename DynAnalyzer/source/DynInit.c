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

void *EventProcess (void* Arg)
{
    while (1)
    {
        QNode *QN = FrontQueue ();
        if (QN == NULL || QN->Flag == FALSE)
        {
            sleep (1);
            continue;
        }
        
        DifEngine (QN->EventId, QN->ThreadId, QN->QBuf);       
        OutQueue ();
    }
    
    return NULL;
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



void TRC_init ()
{
    DWORD Ret;
    pthread_t Tid;

    InitQueue (4096);

    InitDif ();
   
    Ret = pthread_create(&Tid, NULL, EventProcess, NULL);
    assert (Ret == 0);

    DEBUG ("@@@@@ DIFA engine init success!\r\n");
    return;
}


void TRC_exit ()
{
    DEBUG ("@@@@@ Ready to exit, total memory: %u (K)!\r\n", GetPhyMemUse ());
    while (QueueSize ())
    {
        DEBUG ("......\twait for event process...\r\n");
        sleep (1);
    }

    sleep (3);

    WiteGraph ("DIFG");
    DelQueue ();
    DeInitDif ();

    DEBUG ("@@@@@ DIFA engine exits!\r\n");
    return;
}






