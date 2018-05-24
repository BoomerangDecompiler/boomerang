int main(int argc, char *argv[]);

/** address: 0x00010684 */
int main(int argc, char *argv[])
{
    if ((unsigned int)argc > (unsigned int)0xee6b27ff) {
        printf(0x107e8);
    }
    if ((unsigned int)argc <= (unsigned int)0xefffffff) {
        printf("The mask is %x\n", (unsigned int)0xf0000000);
    }
    if ((unsigned int)argc > (unsigned int)1) {
        printf(0x10810);
    }
    if (0 - argc < -2) {
        printf(0x10828);
    }
    return 0;
}

