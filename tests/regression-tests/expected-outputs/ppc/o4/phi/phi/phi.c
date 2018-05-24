int main(int argc, char *argv[]);
__size32 fib(int param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6, __size32 param7, __size32 param8, int param9);

/** address: 0x100004bc */
int main(int argc, char *argv[])
{
    int CR1; 		// r65
    int g10; 		// r10
    int g11; 		// r11
    int g12; 		// r12
    int g3; 		// r3
    int g5; 		// r5
    int g6; 		// r6
    int g7; 		// r7
    int g8; 		// r8
    int local0; 		// m[g1 - 24]

    printf(0x10000930);
    g5 = scanf(0x1000092c); /* Warning: also results in g6, g7, g8, g10, g11, g12 */
    if (local0 > 1) {
        g3 = fib(local0 - 1, g5, g6, g7, g8, g10, g11, g12, 0); /* Warning: also results in g5, g6, g7, g8, g10, g11, g12, CR1 */
        fib(g3 - 1, g5, g6, g7, g8, g10, g11, g12, CR1);
        printf(0x1000092c);
        printf(0x10000940);
    }
    else {
        if (local0 != 1) {
        }
        printf(0x10000940);
    }
    return 0;
}

/** address: 0x10000440 */
__size32 fib(int param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6, __size32 param7, __size32 param8, int param9)
{
    int CR1; 		// r65
    __size32 g1; 		// r1
    __size32 g10; 		// r10
    __size32 g11; 		// r11
    __size32 g12; 		// r12
    __size32 g29; 		// r29
    int g3; 		// r3
    int g3_1; 		// r3{0}
    __size32 g5; 		// r5
    __size32 g6; 		// r6
    __size32 g7; 		// r7
    __size32 g8; 		// r8
    __size32 local10; 		// param8{0}
    int local11; 		// param9{0}
    int local3; 		// g3{0}
    __size32 local4; 		// param2{0}
    __size32 local5; 		// param3{0}
    __size32 local6; 		// param4{0}
    __size32 local7; 		// param5{0}
    __size32 local8; 		// param6{0}
    __size32 local9; 		// param7{0}

    g1 = g1 - 32;
    local4 = param2;
    local4 = param2;
    local5 = param3;
    local5 = param3;
    local6 = param4;
    local6 = param4;
    local7 = param5;
    local7 = param5;
    local8 = param6;
    local8 = param6;
    local9 = param7;
    local9 = param7;
    local10 = param8;
    local10 = param8;
    local11 = param9;
    local11 = param9;
    if (param1 > 1) {
        g3 = fib(param1 - 1, param2, param3, param4, param5, param6, param7, param8, param9); /* Warning: also results in g5, g6, g7, g8, g10, g11, g12, CR1 */
        g1 = fib(g3 - 1, g5, g6, g7, g8, g10, g11, g12, CR1); /* Warning: also results in g29, CR1 */
        r[65] = r[65] & ~0x4;
        g5 = printf(0x1000092c); /* Warning: also results in g6, g7, g8, g10, g11, g12 */
        local4 = g5;
        local5 = g6;
        local6 = g7;
        local7 = g8;
        local8 = g10;
        local9 = g11;
        local10 = g12;
        local11 = CR1;
        g3 = g29;
        g1 += 32;
        local3 = g3;
    }
    else {
        g3_1 = 1;
        local3 = g3_1;
        if (param1 != 1) {
            g3 = param1;
            local3 = g3;
        }
    }
    g3 = local3;
    param2 = local4;
    param3 = local5;
    param4 = local6;
    param5 = local7;
    param6 = local8;
    param7 = local9;
    param8 = local10;
    param9 = local11;
    g29 = *(g1 + 20);
    return g3; /* WARNING: Also returning: g5 := param2, g6 := param3, g7 := param4, g8 := param5, g10 := param6, g11 := param7, g12 := param8, g29 := g29, CR1 := param9 */
}

