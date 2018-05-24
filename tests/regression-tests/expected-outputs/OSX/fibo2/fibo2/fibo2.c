int main(int argc, char *argv[]);
__size32 fib1();
__size32 fib2(int param1);

/** address: 0x00001cf0 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    __size32 g31; 		// r31

    printf(/* machine specific */ (int) LR + 720);
    scanf(/* machine specific */ (int) LR + 736);
    g3 = fib1(); /* Warning: also results in g31 */
    *(__size32*)(g30 + 68) = g3;
    printf(g31 + 740);
    return 0;
}

/** address: 0x00001c3c */
__size32 fib1()
{
    __size32 g29; 		// r29
    int g3; 		// r3
    __size32 g30; 		// r30
    __size32 g31; 		// r31
    __size32 g9; 		// r9
    __size32 tmp + 30; 		// r[tmp + 30]

    g3 = fib2(g3); /* Warning: also results in g9, g29, g30 */
    return g29; /* WARNING: Also returning: g30 := tmp + 30, g30 := g30, g31 := tmp + 30, g31 := g31, g3 := g3, g9 := g9, g29 := g29, g29 := g29, g29 := g29, g29 := g29, g30 := tmp + 30, g30 := g30, g30 := tmp + 30, g30 := g30, g30 := tmp + 30, g30 := g30, g30 := g30, g31 := tmp + 30, g31 := tmp + 30, g31 := tmp + 30, g31 := tmp + 30 */
}

/** address: 0x00001c78 */
__size32 fib2(int param1)
{
    __size32 g1; 		// r1
    __size32 g29; 		// r29
    __size32 g29_1; 		// r29{0}
    __size32 g29_2; 		// r29{0}
    __size32 g29_5; 		// r29{0}
    __size32 g29_6; 		// r29{0}
    int g3; 		// r3
    __size32 g30_1; 		// r30{0}
    __size32 g30_2; 		// r30{0}
    __size32 g30_5; 		// r30{0}
    __size32 g30_6; 		// r30{0}
    __size32 g30_7; 		// r30{0}
    __size32 g9; 		// r9
    __size32 local7; 		// g29_1{0}
    __size32 local8; 		// g30_5{0}
    __size32 local9; 		// g30_6{0}

    g30_1 = g1 - 96;
    local7 = g29;
    local8 = g30_1;
    local9 = g30_1;
    if (param1 <= 1) {
    }
    else {
        fib1();
        g3 = fib1(); /* Warning: also results in g9, g29_2, g30_2 */
        local8 = g30_2;
        local9 = g30_2;
        g29_5 = g29_2 + g3;
        *(__size32*)(g30_2 + 64) = g29_2 + g3;
        local7 = g29_5;
    }
    g29_1 = local7;
    g30_5 = local8;
    g30_6 = local9;
    g3 = *(g30_5 + 64);
    return g29_6; /* WARNING: Also returning: g30 := g30_6, g30 := g30_7, g3 := g3, g9 := g9, g29 := g29_6, g29 := g29_6, g29 := g29_6, g29 := g29_6, g29 := g29_1, g30 := g30_6, g30 := g30_7, g30 := g30_6, g30 := g30_7, g30 := g30_6, g30 := g30_7, g30 := g30_7, g30 := g30_5 */
}

