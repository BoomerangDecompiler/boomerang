int main(int argc, char *argv[]);

/** address: 0x00010a54 */
int main(int argc, char *argv[])
{
    int o0; 		// r8

    if ((unsigned int)(argc - 2) > (unsigned int)5) {
        o0 = 0x11678;
    }
    else {
        switch(argc) {
        case 2:
bb0x10a94:
            o0 = 0x11648;
            goto bb0x10a94;
        case 3:
bb0x10aa0:
            o0 = 0x11650;
            goto bb0x10aa0;
        case 4:
bb0x10aac:
            o0 = 0x11658;
            goto bb0x10aac;
        case 5:
bb0x10ab8:
            o0 = 0x11660;
            goto bb0x10ab8;
        case 6:
bb0x10ac4:
            o0 = 0x11668;
            goto bb0x10ac4;
        case 7:
bb0x10ad0:
            o0 = 0x11670;
            goto bb0x10ad0;
        }
    }
    printf(o0);
    return 0;
}

