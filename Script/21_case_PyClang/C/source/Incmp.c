#include <stdio.h>
#include <string.h>

typedef struct S
{
    char buf[128];
}S;

char* Key = "echo hello world";

int g = 0;


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

    g = strlen (Ctx);
    memcpy (st.buf, Ctx, g);

    return Cmp (&st);
}



