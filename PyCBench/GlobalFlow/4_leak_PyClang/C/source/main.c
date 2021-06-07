#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct S
{
    char ctx[128];
    int length;
}S;

extern char * g;
char* Getpasswd ();
void Trace (S *st);


int main(int argc, char ** argv) 
{
    Getpasswd ();

    S st;

    strcpy (st.ctx, g);

    Trace (&st);

    return 0;
}




