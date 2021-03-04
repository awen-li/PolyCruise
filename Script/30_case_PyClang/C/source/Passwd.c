#include <stdio.h>
#include <stdlib.h>

char *g;

char * Return (char *ctx);


void Retrive ()
{
    g = getenv("CASE1");
}

char* Getpasswd ()
{
    Retrive ();

    return Return (g);
}




