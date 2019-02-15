int main(int argc, char *argv[]);


/** address: 0x00001cc4 */
int main(int argc, char *argv[])
{
    *(__size32*)(/* machine specific */ (int) LR + 856) = 12;
    printf(/* machine specific */ (int) LR + 796);
    printf(/* machine specific */ (int) LR + 808);
    return 0;
}

