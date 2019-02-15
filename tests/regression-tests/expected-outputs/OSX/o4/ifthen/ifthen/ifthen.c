int main(int argc, char *argv[]);


/** address: 0x00001cf4 */
int main(int argc, char *argv[])
{
    puts(/* machine specific */ (int) LR + 744);
    putchar('1');
    if (argc > 3) {
    }
    printf(/* machine specific */ (int) LR + 756);
    return;
}

