// address: 8048450
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    FILE *eax_1; 		// r24{20}
    __size32 edx; 		// r26
    char local0[1024]; 		// m[esp - 0x40c]

    if (argc <= 1) {
        eax = 1;
    } else {
        edx = *(argv + 4);
        fopen(edx, "r");
        eax = 1;
        if (eax_1 != 0) {
            fgets(&local0, 1024, eax_1);
            if (eax != 0) {
                strchr(eax, '\n');
                if (eax != 0) {
                    *(__size8*)eax = 0;
                }
                puts(&local0);
            }
            fclose(eax_1);
        }
    }
    return eax;
}

