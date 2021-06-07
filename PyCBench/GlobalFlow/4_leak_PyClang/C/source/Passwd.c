#include <stdio.h>
#include <stdlib.h>


char *g = NULL;

typedef struct S
{
    char ctx[128];
    int length;
}S;


void Trace (S *st)
{
    printf ("trac : %s \r\n", st->ctx);
    return;
}


void Getpasswd ()
{
	g = getenv("CASE1");
	
	return;
	
}




