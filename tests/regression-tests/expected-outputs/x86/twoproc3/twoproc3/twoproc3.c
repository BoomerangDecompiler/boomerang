int main(int argc, char *argv[]);
__size32 getDevice(__size32 param1);


/** address: 0x0804847f */
int main(int argc, char *argv[])
{
    pciVideoRec *eax; 		// r24
    pciVideoRec **eax_1; 		// r24
    int eax_2; 		// r24

    eax_1 = xf86GetPciVideoInfo();
    eax = *eax_1;
    eax_2 = getDevice(eax);
    printf("%i\n", eax_2);
    return 0;
}

/** address: 0x08048474 */
__size32 getDevice(__size32 param1)
{
    __size32 eax; 		// r24

    eax = *(param1 + 24);
    return eax;
}

