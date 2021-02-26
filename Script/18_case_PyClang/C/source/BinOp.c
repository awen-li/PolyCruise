#include <stdio.h>
#include <string.h>


int g = 10;

int increase (int value)
{
     if  (value > 0)
     {
        return value - g;
     }
     else
     {
        return value + g;
     }
}


int BinOp (int Oper)
{
    int Value = increase (Oper);

    return Value;
}



