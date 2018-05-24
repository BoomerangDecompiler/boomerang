int main(int argc, char *argv[]);

/** address: 0x08048334 */
int main(int argc, char *argv[])
{
    union { int; int *; } eax; 		// r24
    int esp; 		// r28
    int local0; 		// m[esp - 80]

    local0 = 0;
    while (local0 <= 63) {
        eax = esp + local0 - 76;
        *(int*)(esp + local0 - 76) = 0;
        local0++;
    }
    return 0;
}

