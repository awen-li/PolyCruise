#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* Getpasswd ();

extern char * g;

void Trace ()
{
	printf ("trac : %s \r\n", g);
	return;
}

int main(int argc, char ** argv) 
{
	Getpasswd ();

	Trace ();
	
	return 0;
}




