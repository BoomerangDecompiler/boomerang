int main(union { int; char *x34; FILE *; } argc, char *argv[]);
__size32 chomp(char *param1, int param2, FILE *param3);


/** address: 0x080484a3 */
int main(union { int; char *x34; FILE *; } argc, char *argv[])
{
    int eax; 		// r24
    union { int; char *x16; FILE *; } eax_1; 		// r24{12}
    union { int; char *x29; FILE *; } eax_4; 		// r24{14}
    __size32 local0; 		// m[esp - 0x420]
    char local1[]; 		// m[esp - 0x41c]

    if (argc > 1) {
        eax = *(argv + 4);
        eax_1 = fopen(eax, "r");
        if (eax_1 != 0) {
            eax_4 = chomp(&local1, 1024, eax_1);
            if (eax_4 != 0) {
                printf("%s\n", &local1);
            }
            fclose(eax_1);
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
__size32 chomp(char *param1, int param2, FILE *param3)
{
    char *eax_1; 		// r24{4}
    char *eax_4; 		// r24{6}

    eax_1 = fgets(param1, param2, param3);
    if (eax_1 != 0) {
        eax_4 = strchr(eax_1, 10);
        if (eax_4 != 0) {
            *(char*)eax_4 = 0;
        }
    }
    return eax_1;
}

