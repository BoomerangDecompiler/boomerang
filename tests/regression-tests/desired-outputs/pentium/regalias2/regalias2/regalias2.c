int main(int argc, char *argv[]);

/** address: 0x08048370 */
int main(int argc, char *argv[])
{
    int eax; 		// r24

    eax = printf("%08X\n", (0x87654321 >> 8 & 0xffffff | 18) & ~0xff00 | 0x3400);
    return eax;
}

