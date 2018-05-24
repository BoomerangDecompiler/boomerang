int main(int argc, char *argv[]);

/** address: 0x000106a4 */
int main(int argc, char *argv[])
{
    int o0; 		// r8

    if ((unsigned int)argc > (unsigned int)7) {
bb0x106d0:
        o0 = 0x107e8;
        goto bb0x106d0;
    }
bb0x106b8:
    switch(argc) {
    case 0:
        goto bb0x106b8;
    case 1:
        goto bb0x106b8;
    case 2:
bb0x106e8:
        o0 = 0x107f8;
        goto bb0x106e8;
    case 3:
bb0x106f4:
        o0 = 0x10800;
        goto bb0x106f4;
    case 4:
bb0x10718:
        if ((unsigned int)(7 - argc) <= (unsigned int)5) {
bb0x10728:
            switch(7 - argc) {
            case 0:
bb0x106dc:
                o0 = 0x107f0;
                goto bb0x106dc;
            case 1:
bb0x1070c:
                o0 = 0x10810;
                goto bb0x1070c;
            case 2:
bb0x10700:
                o0 = 0x10808;
                goto bb0x10700;
            case 3:
bb0x1073c:
                o0 = 0x10818;
                goto bb0x1073c;
            case 4:
                goto bb0x10728;
            case 5:
                goto bb0x10728;
            }
            goto bb0x10744;
        }
        goto bb0x10718;
    case 5:
        goto bb0x106b8;
    case 6:
        goto bb0x106b8;
    case 7:
        goto bb0x106b8;
    }
bb0x10744:
    puts(o0);
    return 0;
}

