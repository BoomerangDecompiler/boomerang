// address: 8048410
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    int local0; 		// m[esp - 12]
    unsigned int local1; 		// m[esp - 16]

    scanf("%d", &local0);
    scanf("%d", &local1);
    if (5 == local0) {
        eax = *0x8049854;
        fwrite("Equal\n", 1, 6, eax);
        if (5 != local0) {
L23:
            eax = *0x8049854;
            fwrite("Not Equal\n", 1, 10, eax);
        }
    } else {
        goto L23;
    }
    if (5 <= local0) {
L18:
        eax = *0x8049854;
        fwrite("Less or Equal\n", 1, 14, eax);
        if (5 < local0) {
L14:
            eax = *0x8049854;
            fwrite("Less\n", 1, 5, eax);
        } else {
L16:
            eax = *0x8049854;
            fwrite("Greater or Equal\n", 1, 17, eax);
            if (5 < local0) {
                goto L14;
            }
        }
    } else {
        eax = *0x8049854;
        fwrite("Greater\n", 1, 8, eax);
        if (5 > local0) {
            goto L16;
        } else {
            goto L18;
        }
    }
    if (5 <= local1) {
L10:
        eax = *0x8049854;
        fwrite("Less or Equal Unsigned\n", 1, 23, eax);
        if (5 < local1) {
L6:
            eax = *0x8049854;
            fwrite("Carry Set\n", 1, 10, eax);
        } else {
L8:
            eax = *0x8049854;
            fwrite("Carry Clear\n", 1, 12, eax);
            if (5 < local1) {
                goto L6;
            }
        }
    } else {
        eax = *0x8049854;
        fwrite("Greater Unsigned\n", 1, 17, eax);
        if (5 > local1) {
            goto L8;
        } else {
            goto L10;
        }
    }
    if (5 - local0 >= 0) {
        eax = *0x8049854;
        fwrite("Minus\n", 1, 6, eax);
    }
    eax = local0;
    if (5 - local0 < 0) {
        eax = *0x8049854;
        fwrite("Plus\n", 1, 5, eax);
    }
    return eax;
}

