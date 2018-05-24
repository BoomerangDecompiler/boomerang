int main(int argc, char *argv[]);
__size32 fib(int param1);

/** address: 0x00001cf0 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int local0; 		// m[g1 - 32]

    printf(/* machine specific */ (int) LR + 720);
    scanf(/* machine specific */ (int) LR + 736);
    g3 = fib(local0);
    *(__size32*)(g30 + 68) = g3;
    printf(/* machine specific */ (int) LR + 740);
    return 0;
}

/** address: 0x00001c54 */
__size32 fib(int param1)
{
    __size32 g1; 		// r1
    __size32 g30; 		// r30
    int local0; 		// m[g1 - 32]
    int local1; 		// m[g1 - 48]
    int local10; 		// local4{0}
    int local2; 		// m[g1 - 44]
    int local3; 		// m[g1 - 40]
    int local4; 		// m[g1 - 44]{0}
    int local5; 		// m[g1 - 44]{0}

    if (param1 > 1) {
        local1 = 2;
        local2 = 1;
        local3 = 1;
        local10 = local2;
        local4 = local10;
        while (local1 < param1) {
            local5 = local4 + local3;
            local3 = local4;
            local1++;
            local10 = local5;
            local4 = local10;
        }
        local0 = local4;
    }
    else {
        local0 = param1;
    }
    return g30; /* WARNING: Also returning: g3 := local0, g30 := (g1 - 80) */
}

