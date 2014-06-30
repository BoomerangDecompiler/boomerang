// address: 0x106a4
int main(int argc, char *argv[], char *envp[]) {
    int o0; 		// r8

    if ((unsigned int)argc > 7) {
L2:
        o0 = "Other!";
        break;
    }
    switch(argc) {
    case 0:
        goto L2;
    case 1:
        goto L2;
    case 2:
L3:
        o0 = "Two!";
        break;
    case 3:
L4:
        o0 = "Three!";
        break;
    case 4:
        if (7 - argc <= 5) {
            switch(7 - argc) {
            case 0:
L5:
                o0 = "Seven!";
                break;
            case 1:
L6:
                o0 = "Six!";
                break;
            case 2:
L7:
                o0 = "Five!";
                break;
            case 3:
                o0 = "Four!";
                break;
            case 4:
                goto L4;
            case 5:
                goto L3;
            }
            goto L1;
        }
        goto L2;
    case 5:
        goto L7;
    case 6:
        goto L6;
    case 7:
        goto L5;
    }
L1:
    puts(o0);
    return 0;
}

