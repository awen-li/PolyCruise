#include <stdio.h>
#include <stdlib.h>

char *g;

void Retrive ()
{
    g = getenv("CASE1");
}

char* cmp ();


char* Getpasswd ()
{
    Retrive ();

    return cmp ();
}




