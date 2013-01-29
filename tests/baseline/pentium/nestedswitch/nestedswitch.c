// address: 804837c
int main(int argc, char *argv[], char *envp[]) {
    if (argc > 7) {
L2:
        puts("Other!");
        break;
    }
    switch(argc) {
    case 0:
        goto L2;
    case 1:
        goto L2;
    case 2:
L4:
        puts("Two!");
        break;
    case 3:
L6:
        puts("Three!");
        break;
    case 4:
        if (7 - argc <= 5) {
            switch(7 - argc) {
            case 0:
L8:
                puts("Seven!");
                break;
            case 1:
L10:
                puts("Six!");
                break;
            case 2:
L12:
                puts("Five!");
                break;
            case 3:
                puts("Four!");
                break;
            case 4:
                goto L6;
            case 5:
                goto L4;
            }
            goto L0;
        }
        goto L2;
    case 5:
        goto L12;
    case 6:
        goto L10;
    case 7:
        goto L8;
    }
L0:
    return 10 - argc;
}

