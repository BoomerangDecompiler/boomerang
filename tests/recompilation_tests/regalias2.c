

int main()
{
	int r;
	__asm("movl $0x87654321, %%edx; movb $0x12, %%dl; movb $0x34, %%dh" : "=d" (r));
	printf("%08X\n", r);
}

