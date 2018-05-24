int main(int argc, char *argv[]);

/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    if ((unsigned int)argc > (unsigned int)0xee6b27ff) {
        printf(0x10000888);
        if ((unsigned int)argc <= (unsigned int)0xefffffff) {
bb0x10000474:
            printf(0x100008a0);
        }
    }
    else {
        goto bb0x10000474;
    }
    if ((unsigned int)argc > 1) {
        puts(0x100008b0);
    }
    if (0 - argc < -2) {
        puts(0x100008c4);
    }
    return 0;
}

