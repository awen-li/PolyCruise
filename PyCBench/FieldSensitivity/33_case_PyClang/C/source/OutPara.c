#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct
{
    char *p;
    char *v;
}S;

void Pass (S *st, char *Store)
{
    char *tmp = Store;

    strcpy (tmp, st->p);
    printf ("%s\r\n", Store);
    return;
}




