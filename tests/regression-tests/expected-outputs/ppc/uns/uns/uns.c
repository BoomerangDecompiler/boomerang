int main(int argc, union { __size32; char *[] *; } argv);

/** address: 0x10000418 */
int main(int argc, union { __size32; char *[] *; } argv)
{
    if ((unsigned int)argc > (unsigned int)0xee6b27ff) {
        printf(0x10000884);
    }
    if ((unsigned int)argc <= (unsigned int)0xefffffff) {
        printf(0x1000089c);
    }
    if ((unsigned int)argc > 1) {
        printf(0x100008ac);
    }
    if (0 - argc < -2) {
        printf(0x100008c0);
    }
    return 0;
}

