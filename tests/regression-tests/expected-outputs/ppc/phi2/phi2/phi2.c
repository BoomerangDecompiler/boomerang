int main(int argc, union { __size32; char *[] *; } argv);
void proc1(int param1, union { __size32; char[] *; } param2);

/** address: 0x100004f4 */
int main(int argc, union { __size32; char *[] *; } argv)
{
    int g4; 		// r4

    g4 = *(argv + 4);
    proc1(argc, g4);
    printf(0x10000908);
    return 0;
}

/** address: 0x10000440 */
void proc1(int param1, union { __size32; char[] *; } param2)
{
    if (param1 <= 2) {
        strlen(param2);
    }
    else {
        strlen(param2);
        strlen(param2);
        printf(0x100008fc);
    }
    printf(0x10000900);
    return;
}

