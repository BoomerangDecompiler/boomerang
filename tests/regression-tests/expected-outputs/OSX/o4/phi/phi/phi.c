int main(int argc, char *argv[]);
__size32 fib(int param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6, __size32 param7, __size32 param8, __size32 param9);


/** address: 0x00001c54 */
int main(int argc, char *argv[])
{
    int g10; 		// r10
    int g11; 		// r11
    int g12; 		// r12
    int g29; 		// r29
    int g3; 		// r3
    int g5; 		// r5
    int g6; 		// r6
    int g7; 		// r7
    int g8; 		// r8
    int g9; 		// r9
    int local0; 		// m[g1 - 32]

    printf("Input number: ");
    g5 = scanf("%d", &local0); /* Warning: also results in g6, g7, g8, g9, g10, g11, g12 */
    if (local0 <= 1) {
        if (local0 != 1) {
            g5 = local0;
        }
        else {
            g5 = 1;
        }
    }
    else {
        g3 = fib(local0 - 1, g5, g6, g7, g8, g9, g10, g11, g12); /* Warning: also results in g5, g6, g7, g8, g9, g10, g11, g12 */
        g3 = fib(g3 - 1, g5, g6, g7, g8, g9, g10, g11, g12);
        printf("%d", g29 + g3);
        g5 = g29;
    }
    printf("fibonacci(%d) = %d\n", local0, g5);
    return 0;
}

/** address: 0x00001cf8 */
__size32 fib(int param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6, __size32 param7, __size32 param8, __size32 param9)
{
    __size32 g10; 		// r10
    __size32 g11; 		// r11
    __size32 g12; 		// r12
    int g3; 		// r3
    int g3_2; 		// r3{22}
    int g3_5; 		// r3{23}
    __size32 g5; 		// r5
    __size32 g6; 		// r6
    __size32 g7; 		// r7
    __size32 g8; 		// r8
    __size32 g9; 		// r9
    __size32 local10; 		// param8{9}
    __size32 local11; 		// param9{10}
    __size32 local4; 		// param2{3}
    __size32 local5; 		// param3{4}
    __size32 local6; 		// param4{5}
    __size32 local7; 		// param5{6}
    __size32 local8; 		// param6{7}
    __size32 local9; 		// param7{8}

    local4 = param2;
    local5 = param3;
    local6 = param4;
    local7 = param5;
    local8 = param6;
    local9 = param7;
    local10 = param8;
    local11 = param9;
    if (param1 <= 1) {
        if (param1 != 1) {
bb0x1d50:
            param9 = local11;
            param8 = local10;
            param7 = local9;
            param6 = local8;
            param5 = local7;
            param4 = local6;
            param3 = local5;
            param2 = local4;
            g3 = param1;
        }
        else {
            g3 = 1;
        }
    }
    else {
        g3_2 = fib(param1 - 1, param2, param3, param4, param5, param6, param7, param8, param9); /* Warning: also results in g5, g6, g7, g8, g9, g10, g11, g12 */
        g3_5 = fib(g3_2 - 1, g5, g6, g7, g8, g9, g10, g11, g12);
        g5 = printf("%d", g3_2 + g3_5); /* Warning: also results in g6, g7, g8, g9, g10, g11, g12 */
        local11 = g12;
        local10 = g11;
        local9 = g10;
        local8 = g9;
        local7 = g8;
        local6 = g7;
        local5 = g6;
        local4 = g5;
        goto bb0x1d50;
    }
    return g3; /* WARNING: Also returning: g5 := param2, g6 := param3, g7 := param4, g8 := param5, g9 := param6, g10 := param7, g11 := param8, g12 := param9 */
}

