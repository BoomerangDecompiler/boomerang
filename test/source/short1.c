#include "stdio.h"

int test(int a, int b, int c)
{
    if (a < b || b < c)
	    return 1;
    return 0;
}

int main()
{
	printf("Result for 4, 5, 6: %d\n", test(4, 5, 6));
	printf("Result for 6, 5, 4: %d\n", test(6, 5, 4));
	printf("Result for 4, 6, 5: %d\n", test(4, 6, 5));
	return 0;
}
