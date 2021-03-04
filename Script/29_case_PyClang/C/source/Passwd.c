#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *g;
extern char ctx[128];

void Retrive ()
{
    g = getenv("CASE1");
}

void Trans ();
void Leak ();


char* Getpasswd ()
{ 
    Retrive ();

    Trans ();

    Leak ();
    
    return ctx;

}




