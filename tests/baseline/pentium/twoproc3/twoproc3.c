__size32 getDevice(__size32 param1);

// address: 804847f
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    __size32 *eax_1; 		// r24{26}

    xf86GetPciVideoInfo();
    eax = *eax_1;
    eax = getDevice(eax);
    printf("%i\n", eax);
    return 0;
}

// address: 8048474
__size32 getDevice(__size32 param1) {
    __size32 eax; 		// r24

    eax = *(param1 + 24);
    return eax;
}

