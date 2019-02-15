int main(int argc, char *argv[]);

int b = 7;
int a = 5;

/** address: 0x10000498 */
int main(int argc, char *argv[])
{
    b = 12;
    printf("a = %i\n", a);
    printf("b = %i\n", b);
    return 0;
}

