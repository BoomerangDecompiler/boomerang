int main(int argc, char *argv[]);

int b = 7;

/** address: 0x100004a8 */
int main(int argc, char *argv[])
{
    b = 12;
    printf("a = %lld\n", 12);
    printf("b = %i\n", b);
    return 0;
}

