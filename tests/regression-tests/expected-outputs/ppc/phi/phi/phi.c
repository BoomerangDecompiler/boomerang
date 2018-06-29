int main(int argc, char *argv[]);
__size32 fib(int param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6, __size32 param8, __size32 param9, __size32 param9);

/** address: 0x100004fc */
int main(int argc, char *argv[])
{
    int g10; 		// r10
    int g12; 		// r12
    int g3; 		// r3
    int g4; 		// r4
    int g5; 		// r5
    int g6; 		// r6
    int g7; 		// r7
    int g8; 		// r8
    int local0; 		// m[g1 - 24]

    printf("Input number: ");
    g4 = scanf("%d", &local0); /* Warning: also results in g5, g6, g7, g8, g10, g12 */
    g3 = fib(local0, g4, g5, g6, g7, g8, g10, g12, 0);
    printf("fibonacci(%d) = %d\n", local0, g3);
    return 0;
}

/** address: 0x10000440 */
__size32 fib(int param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6, __size32 param8, __size32 param9, __size32 param9)
{
    __size32 CR1; 		// r65
    __size1 CR1_1; 		// r65
    __size32 g10; 		// r10
    __size32 g12; 		// r12
    int g3; 		// r3
    int g3_2; 		// r3{0}
    __size32 g4; 		// r4
    __size32 g5; 		// r5
    __size32 g6; 		// r6
    __size32 g7; 		// r7
    __size32 g8; 		// r8
    __size32 local10; 		// param6{0}
    __size32 local11; 		// param8{0}
    __size32 local12; 		// param9{0}
    __size32 local13; 		// param9{0}
    int local5; 		// m[g1 - 32]
    __size32 local6; 		// param2{0}
    __size32 local7; 		// param3{0}
    __size32 local8; 		// param4{0}
    __size32 local9; 		// param5{0}

    local6 = param2;
    local6 = param2;
    local7 = param3;
    local7 = param3;
    local8 = param4;
    local8 = param4;
    local9 = param5;
    local9 = param5;
    local10 = param6;
    local10 = param6;
    local11 = param8;
    local11 = param8;
    local12 = param9;
    local12 = param9;
    local13 = param9;
    local13 = param9;
    if (param1 <= 1) {
        if (param1 != 1) {
            local5 = param1;
        }
        else {
            local5 = 1;
        }
    }
    else {
        g3_2 = fib(param1 - 1, param2, param3, param4, param5, param6, param8, param9, param9); /* Warning: also results in g4, g5, g6, g7, g8, g10, g12, CR1 */
        g3 = fib(g3_2 - 1, g4, g5, g6, g7, g8, g10, g12, CR1); /* Warning: also results in CR1 */
        r[65] = r[65] & ~0x4;
        g4 = printf("%d", g3_2 + g3); /* Warning: also results in g5, g6, g7, g8, g10, g12 */
        local6 = g4;
        local7 = g5;
        local8 = g6;
        local9 = g7;
        local10 = g8;
        local11 = g10;
        local12 = g12;
        local13 = CR1_1;
        local5 = g3_2;
    }
    param2 = local6;
    param3 = local7;
    param4 = local8;
    param5 = local9;
    param6 = local10;
    param8 = local11;
    param9 = local12;
    param9 = local13;
    return local5; /* WARNING: Also returning: g4 := param2, g5 := param3, g6 := param4, g7 := param5, g8 := param6, g10 := param8, g12 := param9, CR1 := param9 */
}

