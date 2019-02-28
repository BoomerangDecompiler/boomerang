int main(int argc, char *argv[]);

__size32 a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    int g11; 		// r11
    unsigned int g11_1; 		// r11{5}
    unsigned int g11_4; 		// r11{6}
    int g4; 		// r4
    __size32 g4_1; 		// r4{4}
    __size32 g4_2; 		// r4{7}
    __size32 local0; 		// g4_1{4}
    unsigned int local1; 		// g11_1{5}

    g11 = 0;
    local1 = g11;
    g4 = 0;
    local0 = g4;
    do {
        g4_1 = local0;
        g11_1 = local1;
        g11_4 = g11_1 + 1;
        local1 = g11_4;
        g4_2 = g4_1 + a[g11_1];
        local0 = g4_2;
    } while (flags);
    printf("Sum is %d\n", g4_1 + a[g11_1]);
    return 0;
}

