int main(int argc, char *argv[]);
__size32 bswap(int param1);


/** address: 0x0804837a */
int main(int argc, char *argv[])
{
    int eax; 		// r24

    eax = bswap(0x12345678);
    printf("Output is %x\n", eax);
    return 0;
}

/** address: 0x08048370 */
__size32 bswap(int param1)
{
    return (param1 & 0xff) << 24 | (param1 & 0xff00) << 8 | (unsigned int)(param1 & 0xff0000) >> 8 | (unsigned int)(param1 & ~0xffffff) >> 24;
}

