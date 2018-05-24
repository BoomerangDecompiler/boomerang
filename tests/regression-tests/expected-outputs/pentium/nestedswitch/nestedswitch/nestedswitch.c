int main(int argc, char *argv[]);

/** address: 0x0804837c */
int main(int argc, char *argv[])
{
    if ((unsigned int)argc > (unsigned int)7) {
bb0x80483a8:
        puts("Other!");
        goto bb0x80483a8;
    }
bb0x8048397:
    switch(argc) {
    case 0:
        goto bb0x8048397;
    case 1:
        goto bb0x8048397;
    case 2:
        puts("Two!");
bb0x80483b5:
        goto bb0x80483b5;
    case 3:
        puts("Three!");
bb0x8048419:
        goto bb0x8048419;
    case 4:
        if ((unsigned int)(7 - argc) <= (unsigned int)5) {
bb0x8048420:
            switch(7 - argc) {
            case 0:
                puts("Seven!");
bb0x80483dd:
                goto bb0x80483dd;
            case 1:
                puts("Six!");
bb0x80483f1:
                goto bb0x80483f1;
            case 2:
                puts("Five!");
bb0x8048405:
                goto bb0x8048405;
            case 3:
                puts("Four!");
bb0x8048434:
                goto bb0x8048434;
            case 4:
                goto bb0x8048420;
            case 5:
                goto bb0x8048420;
            }
            goto bb0x80483bb;
        }
bb0x80483a6:
        goto bb0x80483a6;
    case 5:
        goto bb0x8048397;
    case 6:
        goto bb0x8048397;
    case 7:
        goto bb0x8048397;
    }
bb0x80483bb:
    return 0;
}

