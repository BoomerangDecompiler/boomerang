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
bb0x80483bf:
            if (argc != 11) {
            }
            else {
bb0x80483cc:
                goto bb0x80483cc;
            }
            goto bb0x80483bf;
        } while (argc <= 11);
    }
    else {
        do {
            if (argc <= 2) {
                if (argc <= 3) {
                    printf("9");
                    local0 = 0x804849c;
                    eax = printf("5");
                }
                else {
bb0x8048361:
                    if (argc > 4) {
                        goto bb0x8048361;
                    }
bb0x8048387:
                    goto bb0x8048387;
                }
            }
            else {
bb0x8048352:
                goto bb0x8048352;
            }
        } while (argc <= 5);
    }
    return 7;
}

