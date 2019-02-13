int main(int argc, char *argv[]);


/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 24]

    local0 = argc;
    if (argc < -2) {
        local0 = -2;
    }
    if (local0 > 3) {
        local0 = 3;
    }
    printf("MinMax adjusted number of arguments is %d\n", local0);
    return 0;
}

