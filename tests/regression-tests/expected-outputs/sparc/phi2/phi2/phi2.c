int main(int argc, union { __size32; char *[] *; } argv);
void proc1(int param1, union { __size32; char[] *; } param2);

/** address: 0x00010760 */
int main(int argc, union { __size32; char *[] *; } argv)
{
    int o1; 		// r9

    o1 = *(argv + 4);
    proc1(argc, o1);
    printf(0x10860);
    return 0;
}

/** address: 0x000106c4 */
void proc1(int param1, union { __size32; char[] *; } param2)
{
    if (param1 <= 2) {
        strlen(param2);
    }
    else {
        strlen(param2);
        strlen(param2);
        printf(0x10850);
    }
    printf(0x10858);
    return;
}

