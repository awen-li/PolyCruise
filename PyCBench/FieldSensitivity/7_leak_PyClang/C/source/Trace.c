#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct S
{
    char* module;
    int a;
}S;

char Buffer[64] = {0};

void Trace (char *Module, int Value)
{
    S st = {0};

    st.module = Module;

    printf ("Come fome module: %s ...\r\n", Module);
    return;   
}




