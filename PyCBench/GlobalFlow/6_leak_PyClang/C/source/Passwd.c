#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char g[128];

void Retrive ()
{
    char *P = getenv("CASE1");
    strcat (g, P);
}

char* Getpasswd (void)
{
    strcpy (g, "pwd:");
    
    Retrive ();

    printf ("Value = %s\r\n", g);
    return g;

}




