#include <stdio.h>
#include <string.h>

char *pwd = "123456";

int Overflow (unsigned Value)
{
    unsigned len = Value - 2;
    printf("pwd  = %s \r\n", pwd+len);


    return 0;
}



