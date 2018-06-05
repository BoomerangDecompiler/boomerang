int main(int argc, char *argv[]);
__size32 fib(__size32 param1, __size32 param1, int param2, __size32 param3);

/** address: 0x00001c94 */
int main(int argc, char *argv[])
{
    __size32 g29; 		// r29
    int g3; 		// r3
    int g30; 		// r30
    __size32 g31; 		// r31
    int local0; 		// m[g1 - 32]

    g31 = /* machine specific */ (int) LR;
    printf(/* machine specific */ (int) LR + 824);
    scanf(/* machine specific */ (int) LR + 840);
    if (local0 <= 1) {
    }
    else {
        g3 = fib(/* machine specific */ (int) LR, local0 - 1, g29); /* Warning: also results in g30, g31 */
        g31 = fib(g31, g30 - 2, g3);
    }
    printf(g31 + 844);
    return 0;
}

/** address: 0x00001d20 */
__size32 fib(__size32 param1, __size32 param1, int param2, __size32 param3)
{
    __size32 g29; 		// r29
    int g3; 		// r3
    int g30; 		// r30
    __size32 g31; 		// r31
    int local5; 		// param2{0}
    __size32 local6; 		// param3{0}

    g30 = param2;
    local5 = param2;
    local6 = param3;
    if (param2 > 1) {
        g3 = fib(g31, param2 - 1, param3); /* Warning: also results in g30 */
        g3 = fib(param1, g30 - 2, g3); /* Warning: also results in g29, g30 */
        local6 = g29;
        g3 = g29 + g3;
        local5 = g3;
    }
    param2 = local5;
    param3 = local6;
    return param1; /* WARNING: Also returning: g30 := param1, g3 := param2, g29 := param1, g29 := param1, g29 := param3, g30 := param1, g30 := param1, g30 := g30, g31 := param1 */
}

