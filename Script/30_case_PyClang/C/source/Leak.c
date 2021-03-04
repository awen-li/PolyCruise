#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char * Return (char *ctx)
{
    char* tgt = strdup (ctx);
    printf ("Value = %s\r\n", tgt);
    return tgt;
}




