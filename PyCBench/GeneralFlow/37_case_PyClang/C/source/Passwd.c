#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


char* Getpasswd (int Index)
{
    int CallIndex = Index%2;
    printf ("---------->>>>> CallIndex = %d \r\n", CallIndex);

    return "pwd";
}




