#include <stdio.h>
#include <stdlib.h>

char* Getpasswd ();

void Trace (char *content)
{
	char *fmt = content;
	printf ("trace: %s \r\n", fmt);
	return;
}

int main(int argc, char ** argv) 
{
	char *passwd = Getpasswd ();
	
	Trace (passwd);
	
	return 0;
}




