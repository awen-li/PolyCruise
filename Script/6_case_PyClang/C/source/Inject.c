#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>


int RunExec (char *Cmd)
{
    int iret = execl(Cmd, Cmd, "-l", "/usr/include/stdio.h",0);  
    printf("Command: %s, iret=%d\n", Cmd, iret);
    
    if (iret==-1) 
    {
        printf("%d:%s\n", errno,strerror(errno));
    }

    return iret;
}



int RunSystem (char *Cmd)
{
    int iret = system(Cmd);
    printf("Command: %s, iret=%d\n", Cmd, iret);
    
    if (iret==-1) 
    {
        printf("%d:%s\n", errno,strerror(errno));
    }
    
    return iret;
}



