int main(int argc, char *argv[]);
__size32 fib(__size32 param1, __size32 param1, int param2, __size32 param3);

/** address: 0x00001c94 */
int main(int argc, char *argv[])
{
    __size64 f28; 		// r60
    __size32 g29; 		// r29
    int g3; 		// r3
    int g30; 		// r30
    int local0; 		// m[g1 - 32]

    printf(/* machine specific */ (int) LR + 824);
    scanf(/* machine specific */ (int) LR + 840);
    if (local0 <= 1) {
    }
    else {
        g3 = fib(f28, local0 - 1, g29); /* Warning: also results in g30, f28 */
        fib(f28, g30 - 2, g3);
    }
    printf(/* machine specific */ (int) LR + 844);
    return 0;
}

/** address: 0x00001d20 */
__size32 fib(__size32 param1, __size32 param1, int param2, __size32 param3)
{
    __size64 f28; 		// r60
    __size32 g29_1; 		// r29{0}
    __size32 g29_2; 		// r29{0}
    __size32 g29_5; 		// r29{0}
    int g3; 		// r3
    int g30; 		// r30
    int g30_1; 		// r30{0}
    int g30_2; 		// r30{0}
    int g30_3; 		// r30{0}
    int g30_4; 		// r30{0}
    int local5; 		// param2{0}
    __size32 local6; 		// g29_1{0}
    int local7; 		// g30_4{0}

    g30_1 = param2;
    local5 = param2;
    local6 = param3;
    local7 = g30_1;
    if (param2 > 1) {
        g3 = fib(f28, param2 - 1, param3); /* Warning: also results in g30_2 */
        g3 = fib(param1, g30_2 - 2, g3); /* Warning: also results in g29_2, g30_3 */
        local6 = g29_2;
        local7 = g30_3;
        g3 = g29_2 + g3;
        local5 = g3;
    }
    param2 = local5;
    g29_1 = local6;
    g30_4 = local7;
    return g29_5; /* WARNING: Also returning: g30 := g30, g3 := param2, g29 := g29_5, g29 := g29_5, g29 := g29_1, g30 := g30, g30 := g30, g30 := g30_4, f28 := param1 */
}

