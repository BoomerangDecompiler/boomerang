

int main()
{
	int r;
	__asm("movl $0x87654321, %%eax; movb $0x12, %%al; movb $0x34, %%ah" : "=a" (r));
	printf("%08X\n", r);
}

