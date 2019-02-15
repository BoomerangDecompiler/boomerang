int main(int argc, char *argv[]);


/** address: 0x08048328 */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    __size32 local0; 		// m[esp - 28]
    unsigned int local1; 		// m[esp - 32]

    eax = 0;
    if (argc <= 2) {
        do {
            if (argc != 11) {
            }
        } while (argc <= 11);
    }
    else {
        do {
            if (argc <= 2) {
                if (argc <= 3) {
bb0x8048390:
                    printf("9");
bb0x804836e:
                    local0 = 0x804849c;
                    eax = printf("5");
                }
                else {
                    if (argc > 4) {
                        goto bb0x804836e;
                    }
                    goto bb0x80483b0;
                }
            }
            else {
                goto bb0x8048390;
            }
bb0x80483b0:
        } while (argc <= 5);
    }
    return 7;
}

