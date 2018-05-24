int main(int argc, char *argv[]);

/** address: 0x100004a8 */
int main(int argc, char *argv[])
{
    *(int*)0x100109d0 = 12;
    printf(0x100008b0);
    printf(0x100008bc);
    return 0;
}

