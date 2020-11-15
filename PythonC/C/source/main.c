#include "add.h"

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		return 0;
	}
	
	printf ("%s + %s = %d\r\n", argv[1], argv[2], add (atoi(argv[1]), atoi(argv[2])));
	
	return 0;
}


