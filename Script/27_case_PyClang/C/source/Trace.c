#include <stdio.h>

int sub (int value)
{
    value--;

    return value;
    
}

void Trace (char *Module, int Value)
{
	char BUF[256];
	
	int Key = sub (Value);
	
	sprintf (BUF, "%s use %d \r\n", Module, Key);
	printf ("%s \r\n", BUF);
    return;   
}




