int main(int argc, char *argv[]);


/** address: 0x10000484 */
int main(int argc, char *argv[])
{
    int g0; 		// r0
    int g5; 		// r5
    int g5_1; 		// r5{12}
    int g5_2; 		// r5{14}
    int g9; 		// r9
    int local0; 		// m[g1 - 8]
    int local1; 		// g5{1}
    int local2; 		// g5_1{12}

    printf("Input number: ");
    scanf("%d", &local0);
    g0 = local0;
    if (local0 > 1) {
        g5 = 1;
        local2 = g5;
        local1 = g5;
        g9 = 1;
        if (local0 > 2) {
            do {
                g5_1 = local2;
                g5_2 = g5_1 + g9;
                local2 = g5_2;
                local1 = g5_2;
                g9 = g5_1;
            } while (local0 != 2);
        }
        g5 = local1;
        g0 = g5;
    }
    printf("fibonacci(%d) = %d\n", local0, g0);
    return 0;
}

