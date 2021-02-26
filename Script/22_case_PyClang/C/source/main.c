#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* Getpasswd ();

extern char * g;

void Trace ()
{
	
	return;
}

int main(int argc, char ** argv) 
{
	Getpasswd ();

	printf ("trac : %s \r\n", g);
	
	return 0;
}




