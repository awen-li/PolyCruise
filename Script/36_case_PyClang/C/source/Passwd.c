#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>



typedef struct
{
    char *p;
    char *v;
}S;

typedef void (*FUNCTION_PTR) (S *, char *);

typedef struct Callback
{
    const char *FuncName;
    FUNCTION_PTR FuncPtr;
}Callback;


void Retrive (S *st)
{
    st->p = getenv("CASE1");
}

void Pass (S *st, char *Store);
void Pass2 (S *st, char *Store);


char* Getpasswd ()
{
    char *Store = (char*) malloc (128);
    assert (Store != NULL);
    
    S st;
    Retrive (&st);

    Callback CBary[] = 
    {
        {"Pass",  (FUNCTION_PTR)Pass},
        {"Pass2", (FUNCTION_PTR)Pass2},
    };

    for (int Index = 0; Index < sizeof (CBary)/sizeof (Callback); Index++)
    {
        Callback* CB = CBary + Index;
        printf ("Invoce : %s \r\n", CB->FuncName);
        CB->FuncPtr (&st, Store);
        printf ("leak --- %s \r\n", Store);
    }
    
    return Store;
}




