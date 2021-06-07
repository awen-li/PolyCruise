#include <stdio.h>

void Trace (char *Module, int Value)
{
	char BUF[256];
	
	int Key = Value << 2;
	
	sprintf (BUF, "%s use %d \r\n", Module, Key);
	printf ("%s \r\n", BUF);
    return;   
}




