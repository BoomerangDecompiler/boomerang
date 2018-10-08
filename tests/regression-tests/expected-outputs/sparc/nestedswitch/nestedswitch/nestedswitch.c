int main(unsigned int argc, char *argv[]);

/** address: 0x000106a4 */
int main(unsigned int argc, char *argv[])
{
    int o0; 		// r8

    if (argc > 7) {
bb0x106d0:
        o0 = 0x107e8;
        break;
    }
    switch(argc) {
    case 0:
        goto bb0x106d0;
    case 1:
        goto bb0x106d0;
    case 2:
bb0x106e8:
        o0 = 0x107f8;
        break;
    case 3:
bb0x106f4:
        o0 = 0x10800;
        break;
    case 4:
        if (7 - argc <= 5) {
            switch(7 - argc) {
            case 0:
bb0x106dc:
                o0 = 0x107f0;
                break;
            case 1:
bb0x1070c:
                o0 = 0x10810;
                break;
            case 2:
bb0x10700:
                o0 = 0x10808;
                break;
            case 3:
                o0 = 0x10818;
                break;
            case 4:
                goto bb0x106f4;
            case 5:
                goto bb0x106e8;
            }
            goto bb0x10744;
        }
        goto bb0x106d0;
    case 5:
        goto bb0x10700;
    case 6:
        goto bb0x1070c;
    case 7:
        goto bb0x106dc;
    }
bb0x10744:
    puts(o0);
    return 0;
}

