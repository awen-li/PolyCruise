#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct S
{
    int a;
    int b;
}S;

char Buffer[64] = {0};

void Trace (char *Module, int Value)
{
	S st = {0};

	st.a = Value;

	memcpy (Buffer, "case test......", st.a);
    return;   
}




