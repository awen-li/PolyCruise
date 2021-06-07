#include <stdio.h>
#include <string.h>

typedef struct S
{
    int a;
    char buf[128];
}S;

char* Key = "echo hello world";


int Cmp (S *st)
{
    if (strncmp (st->buf, Key, strlen(st->buf)) == 0) 
    {
        printf("match <%s, %s> \n", Key, st->buf);
        return 1;
    }

    return 0;
}


int Incmp (char *Ctx)
{
    S st;
    
    memcpy (st.buf, Ctx, strlen (Ctx));

    return Cmp (&st);
}



