int main(int argc, char *argv[]);


/** address: 0x00001d10 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 + 24]

    local0 = argc;
    if (argc < -2) {
        local0 = -2;
    }
    if (local0 <= 3) {
    }
    printf(/* machine specific */ (int) LR + 684);
    return 0;
}

