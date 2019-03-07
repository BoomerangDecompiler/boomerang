int main(int argc, char *argv[]);


/** address: 0x00001c98 */
int main(int argc, char *argv[])
{
    if ((unsigned int)argc > (unsigned int)0xee6b27ff) {
        printf("Population exceeds %u\n", (unsigned int)0xee6b2800);
    }
    if ((unsigned int)argc <= (unsigned int)0xefffffff) {
        printf("The mask is %x\n", (unsigned int)0xf0000000);
    }
    if ((unsigned int)argc > 1) {
        puts("Arguments supplied");
    }
    if (0 - argc < -2) {
        puts("Three or more arguments");
    }
    return 0;
}

