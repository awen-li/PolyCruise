#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef void (*CALL_FUNC) (int Value);

typedef struct
{
    char *Name;
    CALL_FUNC Call;
}CallIt;


void Call_1 (int Value)
{
    // do nothing;
    return;
}

void Call_2 (int Value)
{
    Value++;
    //printf ("&&&&& come to Call_2.... %u \r\n", Value);

    return;
}

CallIt gCallTable[] = 
{
    {"Call_1", Call_1},
    {"Call_2", Call_2},
};


void CallFunc (char *Name, int Vaule)
{
    printf ("====> Name = %s \r\n", Name);
    for (unsigned i = 0; i < sizeof (gCallTable)/sizeof (CallIt); i++)
    {
        CallIt *Ci = gCallTable + i;
        if (strcmp (Ci->Name, Name) == 0)
        {
            Ci->Call (Vaule);
        }    
    }

    return;
}


