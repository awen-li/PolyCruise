#include <stdio.h>
#include <string.h>

static char Buffer[64] = "Here is an example for buffer read!";

int BinOp (int Oper)
{
    int Value = Oper - 4;

    printf ("Buffer -> %s \r\n", Buffer+Value);

    int New = strlen (Buffer)/Oper;

    return New;
}



