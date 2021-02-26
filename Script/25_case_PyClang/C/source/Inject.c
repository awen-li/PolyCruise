#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

char gcmd[128];


int Run1 ()
{
    int iret = execl(gcmd, gcmd, "-l", "/usr/include/stdio.h",0);  
    printf("Command: %s, iret=%d\n", gcmd, iret);
    
    if (iret==-1) 
    {
        printf("%d:%s\n", errno,strerror(errno));
    }

    return iret;
}

int RunExec (char *Cmd)
{
    memcpy (gcmd, Cmd, strlen(Cmd));
    
    return Run1 ();
}




int Run2()
{
    int iret = system(gcmd);
    printf("Command: %s, iret=%d\n", gcmd, iret);
    
    if (iret==-1) 
    {
        printf("%d:%s\n", errno,strerror(errno));
    }
    
    return iret;
}

int RunSystem (char *Cmd)
{
    memcpy (gcmd, Cmd, strlen(Cmd));
    
    return Run2();
}




