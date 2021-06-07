#include <stdio.h>
#include <string.h>

typedef struct S
{
    int a;
    int b;
}S;

char Buffer[64] = {0};

void Trace (int id, int Value)
{
    S st = {2,5};

    st.a = Value;
    st.b = id + 1;

    memcpy (Buffer, "ok", st.b);

    return;   
}




