#include <stdio.h>
#include <stdlib.h>

char *g;

void Retrive ()
{
    g = getenv("CASE1");
}

char* Pass ()
{
    return g;
}

char* Getpasswd ()
{
	Retrive ();
	
	return Pass ();
	
}




