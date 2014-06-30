__size32 test(__size64 param1, int param2);

// address: 0x1d44
int main(int argc, char *argv[], char *envp[]) {
    __size64 f29; 		// r61
    int g3; 		// r3
    __size32 g30; 		// r30

    f29 = test(f29, -5);
    f29 = test(f29, -2);
    g30 = test(f29, 0); /* Warning: also results in f29 */
    g3 = *(g30 + 104);
    f29 = test(f29, g3);
    test(f29, 5);
    return 0;
}

// address: 0x1cd0
__size32 test(__size64 param1, int param2) {
    __size32 g1; 		// r1
    __size32 g30; 		// r30
    int local0; 		// m[g1 + 24]

    local0 = param2;
    if (param2 < -2) {
        local0 = -2;
    }
    if (local0 > 3) {
    }
    printf(/* machine specific */ (int) LR + 772);
    return g30; /* WARNING: Also returning: f29 := param1, g30 := (g1 - 80), f29 := param1 */
}

