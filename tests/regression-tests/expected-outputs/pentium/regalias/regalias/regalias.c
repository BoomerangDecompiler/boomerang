int main(int argc, char *argv[]);


/** address: 0x08048364 */
int main(int argc, char *argv[])
{
    int eax; 		// r24

    eax = printf("%08X\n", (unsigned int)0x87653412);
    return eax;
}

