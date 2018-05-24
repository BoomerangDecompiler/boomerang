int main(int argc, char *argv[]);
void __stat();

/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    __stat();
    printf(0x10000988);
    printf(0x10000994);
    printf(0x100009a0);
    printf(0x100009ac);
    printf(0x100009b8);
    printf(0x100009c4);
    printf(0x100009d0);
    printf(0x100009dc);
    printf(0x100009e8);
    printf(0x100009f4);
    printf(0x10000a04);
    printf(0x10000a10);
    printf(0x10000a1c);
    printf(0x10000a28);
    return 0;
}

/** address: 0x100006dc */
void __stat()
{
    __xstat();
    return;
}

