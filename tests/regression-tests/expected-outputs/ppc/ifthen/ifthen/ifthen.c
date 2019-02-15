int main(int argc, char *argv[]);


/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int local0; 		// m[g1 - 32]

    printf("Figure 19.2\n");
    local0 = 0;
    printf("1");
    if (argc <= 3) {
        local0 = argc;
    }
    g3 = printf("C is %d\n", local0 + argc);
    return g3;
}

