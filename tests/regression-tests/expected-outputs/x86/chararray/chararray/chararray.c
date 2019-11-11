int main(int argc, char *argv[]);


/** address: 0x08048334 */
int main(int argc, char *argv[])
{
    union { int; __size8 *; } eax; 		// r24
    union { int; __size8 *; } esp; 		// r28
    union { int; __size8 *; } local0; 		// m[esp - 80]

    local0 = 0;
    while (local0 <= 63) {
        eax = esp + local0 - 76;
        *(__size8*)(esp + local0 - 76) = 0;
        local0++;
    }
    return 0;
}

