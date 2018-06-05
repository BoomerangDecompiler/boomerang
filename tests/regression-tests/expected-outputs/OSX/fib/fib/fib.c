int main(int argc, char *argv[]);
__size32 fib(__size32 param1, __size32 param1, int param2, __size32 param3, __size32 param4);

/** address: 0x00001ce4 */
int main(int argc, char *argv[])
{
    __size32 g1; 		// r1
    __size32 g29; 		// r29
    __size32 g31; 		// r31
    int g9; 		// r9

    g31 = fib((g1 - 80), 10, g9, g29);
    printf(g31 + 768);
    return 0;
}

/** address: 0x00001d38 */
__size32 fib(__size32 param1, __size32 param1, int param2, __size32 param3, __size32 param4)
{
    __size32 g1; 		// r1
    __size32 g29; 		// r29
    __size32 g29_1; 		// r29{0}
    int g3; 		// r3
    __size32 g30; 		// r30
    __size32 g31; 		// r31
    int g9; 		// r9
    __size32 local7; 		// param3{0}
    __size32 local8; 		// param4{0}

    g30 = g1 - 96;
    local7 = param3;
    local8 = param4;
    if (param2 > 1) {
        g3 = fib(g31, param2 - 1, param2, param4);
        g3 = fib(param1, g9 - 2, g9, g3); /* Warning: also results in g9, g29_1, g30 */
        local7 = g9;
        g29 = g29_1 + g3;
        *(__size32*)(g30 + 64) = g29_1 + g3;
        local8 = g29;
    }
    else {
    }
    param3 = local7;
    param4 = local8;
    g3 = *(g30 + 64);
    return param1; /* WARNING: Also returning: g30 := param1, g3 := g3, g9 := param3, g29 := param1, g29 := param1, g29 := param4, g30 := param1, g30 := param1, g30 := g30, g31 := param1 */
}

