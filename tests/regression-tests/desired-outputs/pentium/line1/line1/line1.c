int main(int argc, char *argv[]);
__size32 chomp(union { char[] *; __size32; } param1, int param2, union { FILE *; __size32; } param3);

/** address: 0x080484a3 */
int main(int argc, char *argv[])
{
    FILE *eax; 		// r24
    int eax_1; 		// r24{0}
    int esp; 		// r28
    __size32 local0; 		// m[esp - 0x420]
    char local1[]; 		// m[esp - 0x41c]

    if (argc > 1) {
        eax = *(argv + 4);
        eax = fopen(eax, "r");
        if (eax != 0) {
            eax_1 = chomp(esp - 1052, 1024, eax);
            if (eax_1 != 0) {
                printf("%s\n", &local1);
            }
            fclose(eax);
        }
        else {
            local0 = 1;
        }
    }
    else {
        local0 = 1;
    }
    return local0;
}

/** address: 0x08048454 */
__size32 chomp(union { char[] *; __size32; } param1, int param2, union { FILE *; __size32; } param3)
{
    char *eax_1; 		// r24{0}
    char *eax_4; 		// r24{0}

    eax_1 = fgets(param1, param2, param3);
    if (eax_1 != 0) {
        eax_4 = strchr(eax_1, '\n');
        if (eax_4 != 0) {
            *(union { char; int; }*)eax_4 = 0;
        }
    }
    return eax_1;
}

