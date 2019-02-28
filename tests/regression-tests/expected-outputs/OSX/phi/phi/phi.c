int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x00001cf0 */
int main(int argc, char *argv[])
{
    __size32 local0; 		// m[g1 - 32]

    printf(/* machine specific */ (int) LR + 724);
    scanf(/* machine specific */ (int) LR + 720);
    fib(local0);
    printf(/* machine specific */ (int) LR + 740);
    return 0;
}

/** address: 0x00001c34 */
__size32 fib(int param1)
{
    int g3; 		// r3
    int local6; 		// m[g1 - 28]

    if (param1 <= 1) {
        if (param1 != 1) {
            local6 = param1;
        }
        else {
            local6 = 1;
        }
    }
    else {
        g3 = fib(param1 - 1);
        fib(g3 - 1);
        printf(/* machine specific */ (int) LR + 908);
        local6 = g3;
    }
    return local6;
}

