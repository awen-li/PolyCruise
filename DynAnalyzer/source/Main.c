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
void DynExit ();


void Exit(int signum)
{
    printf ("Receive signal %u \r\n", signum);
    QueueSetExit();
}


int main(int argc, char ** argv) 
{ 
    signal(SIGINT, Exit);
    signal(SIGTSTP, Exit);

    DynInit ();

    DynExit ();

    return 0;
}

