#include <stdio.h>

int main()
{
	float a=5.; float b;

	scanf("%f", &b);
    printf("a is %f, b is %f\n", a, b);
	if (a == b) printf("Equal\n");
	if (a != b) printf("Not Equal\n");
	if (a >  b) printf("Greater\n");
	if (a <= b) printf("Less or Equal\n");
	if (a >= b) printf("Greater or Equal\n");
	if (a <  b) printf("Less\n");
}
