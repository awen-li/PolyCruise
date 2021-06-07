#include <stdio.h>
#include <string.h>


int g = 10;
char gbuf[128];

int increase (int value)
{
     if  (value > 0)
     {
        strncpy (gbuf, "value > 0", value);
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



