int main(int argc, char *argv[]);

/** address: 0x00010684 */
int main(int argc, char *argv[])
{
    int o0; 		// r8
    int o0_1; 		// r8{0}

    o0 = 0;
    do {
        o0_1 = o0;
        printf(0x10768);
        o0 = o0_1 + 1;
    } while (o0_1 + 1 <= 9);
    printf(0x10770);
    return 0;
}

