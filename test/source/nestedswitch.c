/* Nested switch jump table recognition test */
#include <stdio.h>

int main(int argc, char* argv[])
{
	int n = 10-argc;
	switch(argc)
	{
		case 2:
			printf("Two!\n"); break;
		case 3:
			printf("Three!\n"); break;
		case 4:
			switch (n) {
				case 10-2:
					printf("Two!\n"); break;
				case 10-3:
					printf("Three!\n"); break;
				case 10-4:
					printf("Four!\n"); break;
				case 10-5:
					printf( "Five!\n"); break;
				case 10-6:
					printf( "Six!\n"); break;
				case 10-7:
					printf( "Seven!\n"); break;
				default:
					printf( "Other!\n"); break;
			}
			break;
		case 5:
			printf( "Five!\n"); break;
		case 6:
			printf( "Six!\n"); break;
		case 7:
			printf( "Seven!\n"); break;
		default:
			printf( "Other!\n"); break;
	}
    return 0;
}
