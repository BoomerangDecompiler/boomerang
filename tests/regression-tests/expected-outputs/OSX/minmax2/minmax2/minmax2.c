int main(int argc, char *argv[]);
__size32 test(__size32 param1, int param2);

/** address: 0x00001d44 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    __size32 g30; 		// r30
    __size32 g31; 		// r31

    g31 = test(g31, -5);
    g31 = test(g31, -2);
    g30 = test(g31, 0); /* Warning: also results in g31 */
    g3 = *(g30 + 104);
    g31 = test(g31, g3);
    test(g31, 5);
    return 0;
}

/** address: 0x00001cd0 */
__size32 test(__size32 param1, int param2)
{
    __size32 g1; 		// r1
    int local0; 		// m[g1 + 24]

    local0 = param2;
    if (param2 < -2) {
        local0 = -2;
    }
    if (local0 > 3) {
    }
    printf(/* machine specific */ (int) LR + 772);
    return param1; /* WARNING: Also returning: g31 := param1, g30 := (g1 - 80), g31 := /* machine specific */ (int) LR */
}

