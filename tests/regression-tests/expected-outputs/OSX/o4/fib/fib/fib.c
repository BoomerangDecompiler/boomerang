int main(int argc, char *argv[]);
__size32 fib(int param2, __size32 param2);

/** address: 0x00001d20 */
int main(int argc, char *argv[])
{
    __size32 g29; 		// r29

    fib(10, g29);
    printf(/* machine specific */ (int) LR + 720);
    return 0;
}

/** address: 0x00001d68 */
__size32 fib(int param2, __size32 param2)
{
    __size32 g29_1; 		// r29{0}
    __size32 g29_4; 		// r29{0}
    __size32 g29_5; 		// r29{0}
    int g3; 		// r3
    int g30; 		// r30
    int g30_1; 		// r30{0}
    int g30_2; 		// r30{0}
    int g30_3; 		// r30{0}
    int g30_4; 		// r30{0}
    int local5; 		// param2{0}
    __size32 local6; 		// g29_4{0}
    int local7; 		// g30_4{0}

    g30_3 = param2;
    local5 = param2;
    local6 = param2;
    local7 = g30_3;
    if (param2 > 1) {
        g3 = fib(param2 - 1, param2); /* Warning: also results in g30_1 */
        g3 = fib(g30_1 - 2, g3); /* Warning: also results in g29_1, g30_2 */
        local6 = g29_1;
        local7 = g30_2;
        g3 = g29_1 + g3;
        local5 = g3;
    }
    param2 = local5;
    g29_4 = local6;
    g30_4 = local7;
    return g29_5; /* WARNING: Also returning: g30 := g30, g3 := param2, g29 := g29_5, g29 := g29_5, g29 := g29_4, g30 := g30, g30 := g30, g30 := g30_4 */
}

