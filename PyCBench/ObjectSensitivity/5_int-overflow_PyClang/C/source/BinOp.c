#include <stdio.h>
#include <string.h>


int g = 10;

typedef struct S
{
    int a;
    int b;
}S;


int increase (S *st)
{
     if  (st->a > 0)
     {
        return st->a - g;
     }
     else
     {
        return st->a + g;
     }
}


int BinOp (int Oper)
{
    S st;

    st.a = Oper;
    int Value = increase (&st);

    return Value;
}



