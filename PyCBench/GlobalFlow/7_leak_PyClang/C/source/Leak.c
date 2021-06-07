#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern char *g;
char ctx[128];


void Trans (void)
{
    strcpy (ctx, g);
}

void Leak (void)
{
    char curctx[128];
    
    strcpy (curctx, ctx);
    printf ("Value = %s\r\n", curctx);
}




