int main(int argc, union { double; char *[] *; } argv);
void foo1(union { double; __size32; } param1);
void foo2(double param1);

int b = 7;

/** address: 0x10000498 */
int main(int argc, union { double; char *[] *; } argv)
{
    foo1(argv);
    printf("b = %i\n", b);
    return 0;
}

/** address: 0x10000468 */
void foo1(union { double; __size32; } param1)
{
    foo2(param1);
    return;
}

/** address: 0x10000418 */
void foo2(double param1)
{
    b = 12;
    printf("a = %f\n", param1);
    return;
}

