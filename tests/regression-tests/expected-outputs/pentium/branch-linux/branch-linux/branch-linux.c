int main(int argc, char *argv[]);

/** address: 0x08048410 */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    int local0; 		// m[esp - 12]
    int local1; 		// m[esp - 16]

    scanf("%d", &local0);
    scanf("%d", &local1);
    if (5 == local0) {
        eax = *0x8049854;
        fwrite("Equal\n", 1, 6, eax);
        if (5 != local0) {
bb0x804844e:
            eax = *0x8049854;
            fwrite("Not Equal\n", 1, 10, eax);
        }
        else {
        }
    }
    else {
        goto bb0x804844e;
    }
    if (5 <= local0) {
bb0x80484a4:
        eax = *0x8049854;
        fwrite("Less or Equal\n", 1, 14, eax);
        if (5 < local0) {
bb0x80484f8:
            eax = *0x8049854;
            fwrite("Less\n", 1, 5, eax);
        }
        else {
bb0x80484ce:
            eax = *0x8049854;
            fwrite("Greater or Equal\n", 1, 17, eax);
            if (5 < local0) {
                goto bb0x80484f8;
            }
        }
    }
    else {
        eax = *0x8049854;
        fwrite("Greater\n", 1, 8, eax);
        if (5 > local0) {
            goto bb0x80484ce;
        }
        else {
            goto bb0x80484a4;
        }
    }
    if (5 <= (unsigned int)local1) {
bb0x804854c:
        eax = *0x8049854;
        fwrite("Less or Equal Unsigned\n", 1, 23, eax);
        if (5 < (unsigned int)local1) {
bb0x80485a0:
            eax = *0x8049854;
            fwrite("Carry Set\n", 1, 10, eax);
        }
        else {
bb0x8048576:
            eax = *0x8049854;
            fwrite("Carry Clear\n", 1, 12, eax);
            if (5 < (unsigned int)local1) {
                goto bb0x80485a0;
            }
        }
    }
    else {
        eax = *0x8049854;
        fwrite("Greater Unsigned\n", 1, 17, eax);
        if (5 > (unsigned int)local1) {
            goto bb0x8048576;
        }
        else {
            goto bb0x804854c;
        }
    }
    if (5 >= local0) {
        eax = *0x8049854;
        fwrite("Minus\n", 1, 6, eax);
    }
    eax = local0;
    if (5 < local0) {
        eax = *0x8049854;
        eax = fwrite("Plus\n", 1, 5, eax);
    }
    return eax;
}

