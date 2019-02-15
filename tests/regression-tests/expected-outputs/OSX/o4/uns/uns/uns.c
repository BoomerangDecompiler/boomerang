int main(int argc, char *argv[]);


/** address: 0x00001c98 */
int main(int argc, char *argv[])
{
    if ((unsigned int)argc > (unsigned int)0xee6b27ff) {
        printf(/* machine specific */ (int) LR + 772);
    }
    if ((unsigned int)argc <= (unsigned int)0xefffffff) {
        printf(/* machine specific */ (int) LR + 796);
    }
    if ((unsigned int)argc > 1) {
        puts(/* machine specific */ (int) LR + 812);
    }
    if (0 - argc < -2) {
        puts(/* machine specific */ (int) LR + 832);
    }
    return 0;
}

