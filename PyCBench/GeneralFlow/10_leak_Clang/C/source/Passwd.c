#include <stdio.h>
#include <stdlib.h>

unsigned Getpasswd (void)
{
    char *Value = getenv("CASE1");

    return (unsigned)atoi (Value);
}

void Trace (unsigned *ptr)
{
    printf ("trace: pwd -> %u \r\n", *ptr);
    return;
}




