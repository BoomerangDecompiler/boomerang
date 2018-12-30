int main(int argc, char *argv[]);

/** address: 0x08048410 */
int main(int argc, char *argv[])
{
    FILE *eax; 		// r24
    size_t eax_1; 		// r24
    FILE *eax_10; 		// r24{31}
    FILE *eax_11; 		// r24{34}
    FILE *eax_12; 		// r24{43}
    FILE *eax_2; 		// r24{7}
    FILE *eax_3; 		// r24{10}
    FILE *eax_4; 		// r24{13}
    FILE *eax_5; 		// r24{16}
    FILE *eax_6; 		// r24{19}
    FILE *eax_7; 		// r24{22}
    FILE *eax_8; 		// r24{25}
    FILE *eax_9; 		// r24{28}
    int local0; 		// m[esp - 12]
    unsigned int local1; 		// m[esp - 16]

    scanf("%d", &local0);
    scanf("%d", &local1);
    if (local0 == 5) {
        eax_12 = *0x8049854;
        fwrite("Equal\n", 1, 6, eax_12);
        if (local0 != 5) {
bb0x804844e:
            eax_2 = *0x8049854;
            fwrite("Not Equal\n", 1, 10, eax_2);
        }
    }
    else {
        goto bb0x804844e;
    }
    if (5 <= local0) {
bb0x80484a4:
        eax_4 = *0x8049854;
        fwrite("Less or Equal\n", 1, 14, eax_4);
        if (5 < local0) {
bb0x80484f8:
            eax_6 = *0x8049854;
            fwrite("Less\n", 1, 5, eax_6);
        }
        else {
bb0x80484ce:
            eax_5 = *0x8049854;
            fwrite("Greater or Equal\n", 1, 17, eax_5);
            if (5 < local0) {
                goto bb0x80484f8;
            }
        }
    }
    else {
        eax_3 = *0x8049854;
        fwrite("Greater\n", 1, 8, eax_3);
        if (5 > local0) {
            goto bb0x80484ce;
        }
        else {
            goto bb0x80484a4;
        }
    }
    if (5 <= local1) {
bb0x804854c:
        eax_8 = *0x8049854;
        fwrite("Less or Equal Unsigned\n", 1, 23, eax_8);
        if (5 < local1) {
bb0x80485a0:
            eax_10 = *0x8049854;
            fwrite("Carry Set\n", 1, 10, eax_10);
        }
        else {
bb0x8048576:
            eax_9 = *0x8049854;
            fwrite("Carry Clear\n", 1, 12, eax_9);
            if (5 < local1) {
                goto bb0x80485a0;
            }
        }
    }
    else {
        eax_7 = *0x8049854;
        fwrite("Greater Unsigned\n", 1, 17, eax_7);
        if (5 > local1) {
            goto bb0x8048576;
        }
        else {
            goto bb0x804854c;
        }
    }
    if (5 >= local0) {
        eax_11 = *0x8049854;
        fwrite("Minus\n", 1, 6, eax_11);
    }
    eax_1 = local0;
    if (5 < local0) {
        eax = *0x8049854;
        eax_1 = fwrite("Plus\n", 1, 5, eax);
    }
    return eax_1;
}

