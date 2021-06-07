#include <stdio.h>
#include <string.h>

static char Buffer[64] = "Here is an example for buffer read!";
static char Target[32] = "";

int BinOp (int Oper)
{
    memcpy (Target, Buffer, Oper);

    return strlen (Target)/2;
}



