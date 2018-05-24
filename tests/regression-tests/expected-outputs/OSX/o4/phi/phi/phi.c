int main(int argc, char *argv[]);
__size32 fib(__size32 param1, __size32 param1, int param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6, __size32 param7, __size32 param8, __size32 param9, __size32 param10);

/** address: 0x00001c54 */
int main(int argc, char *argv[])
{
    __size64 f29; 		// r61
    int g10; 		// r10
    int g11; 		// r11
    int g12; 		// r12
    int g3; 		// r3
    __size32 g31; 		// r31
    int g5; 		// r5
    int g6; 		// r6
    int g7; 		// r7
    int g8; 		// r8
    int g9; 		// r9
    int local0; 		// m[g1 - 32]

    g31 = /* machine specific */ (int) LR;
    printf(/* machine specific */ (int) LR + 888);
    g5 = scanf(/* machine specific */ (int) LR + 904); /* Warning: also results in g6, g7, g8, g9, g10, g11, g12 */
    if (local0 <= 1) {
        if (local0 != 1) {
        }
        else {
        }
    }
    else {
        g3 = fib(f29, local0 - 1, g5, g6, g7, g8, g9, g10, g11, g12); /* Warning: also results in g5, g6, g7, g8, g9, g10, g11, g12, f29 */
        g31 = fib(f29, g3 - 1, g5, g6, g7, g8, g9, g10, g11, g12);
        printf(g31 + 904);
    }
    printf(g31 + 908);
    return 0;
}

/** address: 0x00001cf8 */
__size32 fib(__size32 param1, __size32 param1, int param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6, __size32 param7, __size32 param8, __size32 param9, __size32 param10)
{
    __size64 f29; 		// r61
    __size32 g10; 		// r10
    __size32 g11; 		// r11
    __size32 g12; 		// r12
    int g3; 		// r3
    int g30_1; 		// r30{0}
    int g30_4; 		// r30{0}
    int g30_5; 		// r30{0}
    int g30_6; 		// r30{0}
    int g30_7; 		// r30{0}
    __size32 g30_8; 		// r30{0}
    __size32 g31_1; 		// r31{0}
    __size32 g31_4; 		// r31{0}
    __size32 g31_7; 		// r31{0}
    __size32 g31_8; 		// r31{0}
    __size32 g31_9; 		// r31{0}
    __size32 g5; 		// r5
    __size32 g6; 		// r6
    __size32 g7; 		// r7
    __size32 g8; 		// r8
    __size32 g9; 		// r9
    __size32 local10; 		// param9{0}
    __size32 local11; 		// param10{0}
    int local12; 		// g30_7{0}
    int local13; 		// g30_6{0}
    __size32 local14; 		// g31_8{0}
    int local15; 		// g30_5{0}
    __size32 local16; 		// g31_7{0}
    __size32 local4; 		// param3{0}
    __size32 local5; 		// param4{0}
    __size32 local6; 		// param5{0}
    __size32 local7; 		// param6{0}
    __size32 local8; 		// param7{0}
    __size32 local9; 		// param8{0}

    g30_1 = param2;
    g31_1 = /* machine specific */ (int) LR;
    local4 = param3;
    local5 = param4;
    local6 = param5;
    local7 = param6;
    local8 = param7;
    local9 = param8;
    local10 = param9;
    local11 = param10;
    local12 = g30_1;
    local13 = g30_1;
    local14 = g31_1;
    local15 = g30_1;
    local16 = g31_1;
    if (param2 <= 1) {
        if (param2 != 1) {
            param3 = local4;
            param4 = local5;
            param5 = local6;
            param6 = local7;
            param7 = local8;
            param8 = local9;
            param9 = local10;
            param10 = local11;
            g30_7 = local12;
            g30_6 = local13;
            g31_8 = local14;
            g3 = g30_7;
            local15 = g30_6;
            local16 = g31_8;
        }
        else {
            g3 = 1;
        }
    }
    else {
        g3 = fib(f29, param2 - 1, param3, param4, param5, param6, param7, param8, param9, param10); /* Warning: also results in g5, g6, g7, g8, g9, g10, g11, g12 */
        g30_4 = fib(param1, g3 - 1, g5, g6, g7, g8, g9, g10, g11, g12); /* Warning: also results in g31_4 */
        local12 = g30_4;
        local13 = g30_4;
        local14 = g31_4;
        g5 = printf(g31_4 + 740); /* Warning: also results in g6, g7, g8, g9, g10, g11, g12 */
        local4 = g5;
        local5 = g6;
        local6 = g7;
        local7 = g8;
        local8 = g9;
        local9 = g10;
        local10 = g11;
        local11 = g12;
bb0x1d40:
        goto bb0x1d40;
    }
    g30_5 = local15;
    g31_7 = local16;
    return g30_8; /* WARNING: Also returning: g31 := g31_9, g3 := g3, g5 := param3, g6 := param4, g7 := param5, g8 := param6, g9 := param7, g10 := param8, g11 := param9, g12 := param10, g30 := g30_8, g30 := g30_8, g30 := g30_5, g31 := g31_9, g31 := g31_9, g31 := g31_7, f29 := param1 */
}

