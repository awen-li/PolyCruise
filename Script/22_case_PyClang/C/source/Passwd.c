#include <stdio.h>
#include <stdlib.h>


char g[128];

void Getpasswd ()
{
    char *env = getenv("CASE1");
    strcpy (g, env);

    return;
}




