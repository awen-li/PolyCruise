#include <stdio.h>
#include <string.h>

static char Buffer[64] = "Here is an example for buffer read!";
static char Target[32] = "";


int BinOp (int Oper)
{
    int Value = Oper;

    printf ("Buffer -> %s \r\n", Buffer+Value);

    int New = strlen (Buffer)/Oper;

    memcpy (Target, Buffer, Oper);
    printf ("Target -> %s \r\n", Target);

    memset (Buffer, 0, sizeof (Buffer));

    
    return New;
}



