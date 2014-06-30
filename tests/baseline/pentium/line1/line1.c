union { char[] x1; __size8 x2; } *chomp(char param1[], int param2, FILE *param3);

// address: 0x80484a3
int main(int argc, char *argv[], char *envp[]) {
    char *eax; 		// r24
    FILE *eax_1; 		// r24
    FILE *eax_2; 		// r24{26}
    char local0[]; 		// m[esp - 0x41c]
    __size32 local1; 		// m[esp - 0x420]

    if (argc > 1) {
        eax = *(argv + 4);
        fopen(eax, "r");
        if (eax_2 != 0) {
            eax_1 = chomp(&local0, 1024, eax_2);
            if (eax_1 != 0) {
                printf("%s\n", &local0);
            }
            fclose(eax_2);
        } else {
            local1 = 1;
        }
    } else {
        local1 = 1;
    }
    return local1;
}

// address: 0x8048454
union { char[] x1; __size8 x2; } *chomp(char param1[], int param2, FILE *param3) {
    union { char[] x5; __size8 x6; __size8 x4; __size8 x2; } *eax; 		// r24
    union { char[] x1; __size8 x2; } *eax_1; 		// r24{16}

    fgets(param1, param2, param3);
    if (eax_1 != 0) {
        strchr(eax_1, '\n');
        if (eax != 0) {
            *(union { union { char[] x9; __size8 x10; __size8 x8; } x5; __size8 x6; __size8 x4; __size8 x2; }*)eax = 0;
        }
    }
    return eax_1;
}

