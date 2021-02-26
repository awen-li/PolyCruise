#include <stdio.h>
#include <string.h>

typedef struct S
{
    int a;
    int b;
}S;

char Buffer[64] = {0};

void Trace (char *Module, int Value)
{
	S st = {2,5};

	st.a = Value+1;

    int v = st.b/st.a;
    if (v > 0)
    {
        memcpy (Buffer, "ok", v+1);
    }
    return;   
}




