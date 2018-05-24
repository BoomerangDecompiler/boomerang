int main(int argc, char *argv[]);
__size32 fib(int param1, __size32 param2);

/** address: 0x100004c4 */
int main(int argc, char *argv[])
{
    int g9; 		// r9
    int local0; 		// m[g1 - 24]

    printf(0x100008e0);
    g9 = scanf(0x100008f0);
    fib(local0, g9);
    printf(0x100008f4);
    return 0;
}

/** address: 0x10000440 */
__size32 fib(int param1, __size32 param2)
{
    int g3; 		// r3
    int g3_1; 		// r3{0}
    int g9; 		// r9
    int local5; 		// m[g1 - 20]
    __size32 local6; 		// param2{0}

    local6 = param2;
    if (param1 <= 1) {
        local5 = param1;
    }
    else {
        g3_1 = fib(param1 - 1, param1);
        g3 = fib(param1 - 2, param1); /* Warning: also results in g9 */
        local6 = g9;
        local5 = g3_1 + g3;
    }
    param2 = local6;
    return local5; /* WARNING: Also returning: g9 := param2 */
}

