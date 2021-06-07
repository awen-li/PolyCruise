#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    char *p;
    char *v;
}S;

void Retrive (S *st)
{
    st->p = getenv("CASE1");
}

char* Pass (S *st)
{
    return st->p;
}

char* Getpasswd (void)
{
    S st;
	Retrive (&st);
	
	return Pass (&st);
	
}




