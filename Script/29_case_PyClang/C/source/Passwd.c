#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *g;
extern char ctx[128];

void Retrive (void)
{
    g = getenv("CASE1");
}

void Trans (void);
void Leak (void);


char* Getpasswd (void)
{ 
    Retrive ();

    Trans ();

    Leak ();
    
    return ctx;

}




