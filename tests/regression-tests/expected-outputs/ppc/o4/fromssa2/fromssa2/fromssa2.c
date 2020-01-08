int main(int argc, char *argv[]);


/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    int g9; 		// r9
    __size32 g9_1; 		// r9{2}
    __size32 g9_2; 		// r9{4}
    __size32 local0; 		// g9_1{2}

    g9 = 0;
    local0 = g9;
    do {
        g9_1 = local0;
        printf("%d ", g9_1 + 1);
        g9_2 = g9_1 + 1;
        local0 = g9_2;
    } while (g9_1 + 1 <= 9);
    printf("a is %d, x is %d\n", g9_1 + 1, g9_1 + 1);
    return 0;
}

