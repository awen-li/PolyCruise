#include <stdio.h>
#include <stdlib.h>

char* Getpasswd (void)
{
	char *Value = getenv("CASE1");
	
	printf ("Value = %s\r\n", Value);
	return Value;
	
}




