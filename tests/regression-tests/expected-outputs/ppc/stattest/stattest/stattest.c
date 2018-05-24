int main(int argc, char *argv[]);
__size32 __stat();

/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    int g3; 		// r3

    g3 = __stat();
    printf(0x10000894);
    return g3;
}

/** address: 0x100005d0 */
__size32 __stat()
{
    int g3; 		// r3

    g3 = __xstat();
    return g3;
}

