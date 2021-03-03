/***********************************************************
 * Author: Wen Li
 * Date  : 2/10/2021
 * Describe: difaEngine   
 * History:
   <1> 2/10/2021, create
************************************************************/
#include <signal.h>
#include "Queue.h"

void DynInit ();
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

    char* CaseResult = NULL;
    while((ch = getopt(argc, argv, "c:h")) != -1)
    {
        switch(ch)
        {
            case 'c':
            {
                CaseResult = optarg;
                break;
            }
            default:
            {
                return 0;
            }
        }
    }

    DynInit ();

    DynExit (CaseResult);

    return 0;
}

