int main(int argc, char *argv[]);
void fib(int param1);

/** address: 0x100004e0 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 24]

    printf(0x100008fc);
    scanf(0x1000090c);
    fib(local0);
    printf(0x10000910);
    return 0;
}

/** address: 0x10000440 */
void fib(int param1)
{
    int local0; 		// m[g1 - 36]
    int local1; 		// m[g1 - 32]
    int local2; 		// m[g1 - 28]
    int local3; 		// m[g1 - 32]{0}
    int local4; 		// m[g1 - 32]{0}
    int local8; 		// local3{0}

    if (param1 > 1) {
        local0 = 2;
        local1 = 1;
        local2 = 1;
        local8 = local1;
        local3 = local8;
        while (local0 < param1) {
            local4 = local3 + local2;
            local2 = local3;
            local0++;
            local8 = local4;
            local3 = local8;
        }
    }
    else {
    }
    return;
}

