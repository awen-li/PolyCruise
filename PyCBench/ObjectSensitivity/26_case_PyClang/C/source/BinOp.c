#include <stdio.h>
#include <string.h>

int g = 0;

void gChange (int i)
{
    g = g + i;
}


int BinOp (int Oper)
{
    int Value = Oper;

    gChange (Value);

    printf ("g = %u \r\n", Value);
   
    return g;
}



