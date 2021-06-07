#include <stdio.h>
#include <string.h>


int g = 10;

int increase ()
{
    int Val = 0;
    if  (g > 0)
    {
        Val = g--;
    }
    else
    {
        Val = g++;
    }

    return Val;
}


int BinOp (int Oper)
{
    g = Oper;
    int Value = increase ();

    return Value;
}



