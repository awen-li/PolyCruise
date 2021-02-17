#include <stdio.h>
#include <string.h>

int Incmp (char *Ctx)
{
	char* Key = "echo hello world";

    if (strncmp (Ctx, Key, strlen (Ctx)) == 0) 
    {
        printf("match <%s, %s> \n", Key, Ctx);
        return 1;
    }

    return 0;
}



