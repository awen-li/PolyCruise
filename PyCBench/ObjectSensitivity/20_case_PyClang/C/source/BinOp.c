#include <stdio.h>
#include <string.h>

int gOp = 0;

void intOf ()
{
    gOp++;
}


int get ()
{
    return gOp;
}


int BinOp (int Oper)
{
    gOp = Oper;
    intOf ();

    
    return get ();
}



