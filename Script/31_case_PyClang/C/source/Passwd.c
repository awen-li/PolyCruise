#include <stdio.h>
#include <stdlib.h>

char *g;

void Retrive (void)
{
    g = getenv("CASE1");
}

char* cmp (void);


char* Getpasswd (void)
{
    Retrive ();

    return cmp ();
}




