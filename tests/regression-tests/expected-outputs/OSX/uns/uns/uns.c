int main(int argc, char *argv[]);


/** address: 0x00001c94 */
int main(int argc, char *argv[])
{
    if ((unsigned int)argc > (unsigned int)0xee6b27ff) {
        printf(/* machine specific */ (int) LR + 764);
    }
    if ((unsigned int)argc <= (unsigned int)0xefffffff) {
        printf(/* machine specific */ (int) LR + 788);
    }
    if ((unsigned int)argc > 1) {
        printf(/* machine specific */ (int) LR + 804);
    }
    if (0 - argc < -2) {
        printf(/* machine specific */ (int) LR + 824);
    }
    return 0;
}

