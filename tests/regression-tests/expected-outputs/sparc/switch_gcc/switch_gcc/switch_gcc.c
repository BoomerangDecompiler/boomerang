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
            o0 = 0x11648;
            goto bb0x10ae4;
        case 3:
            o0 = 0x11650;
            goto bb0x10ae4;
        case 4:
            o0 = 0x11658;
            goto bb0x10ae4;
        case 5:
            o0 = 0x11660;
            goto bb0x10ae4;
        case 6:
            o0 = 0x11668;
            goto bb0x10ae4;
        case 7:
            o0 = 0x11670;
            goto bb0x10ae4;
        }
    }
bb0x10ae4:
    printf(o0);
    return 0;
}

