/***********************************************************
 * Author: Wen Li
 * Date  : 2/10/2021
 * Describe: difaEngine   
 * History:
   <1> 2/10/2021, create
************************************************************/
#include <signal.h>
#include "Queue.h"

void DynStart ();
void DynExit (char *CaseResult);


void Exit(int signum)
{
    printf ("Receive signal %u \r\n", signum);
    QueueSetExit();
}


int main(int argc, char ** argv) 
{ 
    signal(SIGINT, Exit);
    signal(SIGTSTP, Exit);

    char ch;
    DWORD CrossFlag = FALSE;

    char* CaseResult = NULL;
    while((ch = getopt(argc, argv, "c:sh")) != -1)
    {
        switch(ch)
        {
            case 'c':
            {
                CaseResult = optarg;
                break;
            }
            case 's':
            {
                CrossFlag = TRUE;
                break;
            }
            default:
            {
                return 0;
            }
        }
    }

    DynStart (CrossFlag);

    DynExit (CaseResult);

    return 0;
}

