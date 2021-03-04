#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern char *g;
char pwd[128] = "default";

char* cmp ()
{
    if (strncmp (pwd, g, strlen(g)) != 0)
    {
        strcpy (pwd, g);
    }

    return pwd;
}





