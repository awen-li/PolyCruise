#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct
{
    char *p;
    char *v;
}S;

void Retrive (S *st)
{
    st->p = getenv("CASE1");
}

void Pass (S *st, char *Store);

char* Getpasswd (void)
{
    char *Store = (char*) malloc (128);
    assert (Store != NULL);
    
    S st;
    Retrive (&st);

    Pass (&st, Store);
    printf ("leak --- %s \r\n", Store);
    
    return Store;
}




