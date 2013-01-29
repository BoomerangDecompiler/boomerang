// address: 1066c
int main(int argc, char *argv[], char *envp[]) {
    char *o0; 		// r8

    o0 = "Other!";
    if (argc <= 1) {
L2:
        puts(o0);
    } else {
        if ((unsigned int)(argc - 2 & 0x7) > 7) {
        } else {
            switch(argc - 2 & 0x7) {
            case 0:
                o0 = "Two!";
                goto L2;
            case 1:
                o0 = "Three!";
                goto L2;
            case 2:
                o0 = "Four!";
                goto L2;
            case 3:
                o0 = "Five!";
                goto L2;
            case 4:
                o0 = "Six!";
                goto L2;
            case 5:
L9:
                o0 = "Seven or more!";
                goto L2;
            case 6:
                goto L9;
            case 7:
                goto L9;
            }
            goto L2;
        }
    }
    return 0;
}

