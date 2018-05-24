int main(int argc, char *argv[]);
__size32 fib(__size32 param1, int param2);

/** address: 0x000106fc */
int main(int argc, char *argv[])
{
    int local0; 		// m[o6 - 20]

    printf(0x107f8);
    scanf(0x10808);
    fib(0x10800, local0);
    printf(0x10810);
    return 0;
}

/** address: 0x000106ac */
__size32 fib(__size32 param1, int param2)
{
    int g0; 		// r0
    __size32 g1; 		// r1
    __size32 local3; 		// param1{0}
    __size32 o2; 		// r10
    __size32 o2_1; 		// r10{0}
    __size32 o2_4; 		// r10{0}

    g0 = param2 - 1;
    local3 = param1;
    if (param2 <= 1) {
        o2 = param2;
    }
    else {
        g1 = fib(param1, param2 - 1); /* Warning: also results in o2_1 */
        g0 = fib(g1, param2 - 2); /* Warning: also results in o2_4 */
        g1 = o2_1;
        o2 = o2_4 + o2_1;
        local3 = g1;
    }
    param1 = local3;
    return g0; /* WARNING: Also returning: g1 := param1, o2 := o2 */
}

