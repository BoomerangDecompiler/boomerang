/* Mainly for sparc, ppc and other processors with andn style operators (NAND) */
/* Note: it's somewhat pot luck whether the andn etc instructions get used.
   This worked in with gcc 3.1 for SPARC */
/* Compile with gcc -O1 */
#include <stdio.h>
int main() {
	int a = 0x55AA55AA;
	int b = 0x12345678;
	int c = 0x98765432;
	printf("a andn b is %d\n", a & ~b);
	printf("b orn  a is %d\n",  b | ~a);
	printf("a xorn c is %d\n", a ^ ~c);
	return 0;
}

