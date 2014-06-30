// address: 0x8048948
int main(int argc, char *argv[], char *envp[]) {
    int eax; 		// r24
    int local0; 		// m[esp - 8]
    unsigned int local1; 		// m[esp - 12]

    scanf("%d", &local0);
    scanf("%d", &local1);
    if (local0 == 5) {
        printf("Equal\n");
    }
    if (local0 == 5) {
L19:
        printf("Less or Equal\n");
        if (local0 > 5) {
L15:
            printf("Less\n");
        } else {
L17:
            printf("Greater or Equal\n");
            if (local0 > 5) {
                goto L15;
            }
        }
    } else {
        printf("Not Equal\n");
        if (local0 >= 5) {
            goto L19;
        } else {
            printf("Greater\n");
            if (local0 < 5) {
                goto L17;
            } else {
                goto L19;
            }
        }
    }
    if (local1 >= 5) {
L10:
        printf("Less or Equal Unsigned\n");
        if (local1 > 5) {
L6:
            printf("Carry Set\n");
        } else {
L8:
            printf("Carry Clear\n");
            if (local1 > 5) {
                goto L6;
            }
        }
    } else {
        printf("Greater Unsigned\n");
        if (local1 < 5) {
            goto L8;
        } else {
            goto L10;
        }
    }
    eax = 5 - local0;
    if (eax < 0) {
L1:
        printf("Plus\n");
    } else {
        printf("Minus\n");
        eax = 5 - local0;
        if (eax < 0) {
            goto L1;
        }
    }
    return 0;
}

