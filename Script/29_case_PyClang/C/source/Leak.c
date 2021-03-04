#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern char *g;
char ctx[128];


void Trans ()
{
    strcpy (ctx, g);
}

void Leak ()
{
    char curctx[128];
    
    strcpy (curctx, ctx);
    printf ("Value = %s\r\n", curctx);
}




