#include <stdio.h>
#include <stdlib.h>

unsigned Getpasswd (void);
void Trace (unsigned *ptr);

int main(int argc, char ** argv) 
{
    unsigned pwd = 0;
    unsigned *ptr = &pwd;
    
    pwd = Getpasswd ();
    int num = 0;
    while (num < argc+3)
    {
        pwd = pwd <<1 | num;

        num++;
    }

    Trace (ptr);

    return pwd;
}




