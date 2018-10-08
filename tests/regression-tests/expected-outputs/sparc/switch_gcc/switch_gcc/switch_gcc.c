int main(int argc, char *argv[]);

/** address: 0x00010a54 */
int main(int argc, char *argv[])
{
    int o0; 		// r8

    if ((unsigned int)(argc - 2) > 5) {
        o0 = 0x11678;
    }
    else {
        switch(argc) {
        case 2:
            o0 = 0x11648;
            break;
        case 3:
            o0 = 0x11650;
            break;
        case 4:
            o0 = 0x11658;
            break;
        case 5:
            o0 = 0x11660;
            break;
        case 6:
            o0 = 0x11668;
            break;
        case 7:
            o0 = 0x11670;
            break;
        }
    }
    printf(o0);
    return 0;
}

