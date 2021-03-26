#include <stdio.h>
#include <stdlib.h>

char *g;

char * Return (char *ctx);


void Retrive (void)
{
    g = getenv("CASE1");
}

char* Getpasswd (void)
{
    Retrive ();

    return Return (g);
}




