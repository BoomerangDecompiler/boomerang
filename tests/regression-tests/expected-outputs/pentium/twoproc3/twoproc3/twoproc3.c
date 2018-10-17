int main(int argc, char *argv[]);
__size32 getDevice(__size32 param1);

/** address: 0x0804847f */
int main(int argc, char *argv[])
{
    union { pciVideoRec *; __size32; } *eax; 		// r24
    int eax_1; 		// r24{3}

    eax = xf86GetPciVideoInfo();
    eax = *eax;
    eax_1 = getDevice(eax);
    printf("%i\n", eax_1);
    return 0;
}

/** address: 0x08048474 */
__size32 getDevice(__size32 param1)
{
    __size32 eax; 		// r24

    eax = *(param1 + 24);
    return eax;
}

