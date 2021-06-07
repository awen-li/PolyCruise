#include <stdio.h>
#include <string.h>

static char Buffer[64] = "Here is an example for buffer read!";

typedef struct S
{
    int a;
    int b;
}S;



int BinOp (int Oper)
{
    S st;

    st.a = Oper-2;

    printf ("Buffer -> %s \r\n", Buffer+st.a);

    int New = strlen (Buffer)/st.a;
    
    return New;
}



