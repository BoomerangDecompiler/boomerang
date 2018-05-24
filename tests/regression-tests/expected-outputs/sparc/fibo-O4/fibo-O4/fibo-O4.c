int main(int argc, char *argv[]);
__size32 fib(int param1);

/** address: 0x00010ad0 */
int main(int argc, char *argv[])
{
    int local0; 		// m[o6 - 20]

    printf(0x11688);
    scanf(0x11698);
    if (local0 > 1) {
        fib(local0 - 1);
        fib(local0 - 2);
    }
    printf(0x116a0);
    return 0;
}

/** address: 0x00010a9c */
__size32 fib(int param1)
{
    int i0; 		// r24
    int o0; 		// r8
    int o0_1; 		// r8{0}

    i0 = param1;
    if (param1 > 1) {
        o0_1 = fib(param1 - 1);
        o0 = fib(param1 - 2);
        i0 = o0_1 + o0;
    }
    return i0;
}

