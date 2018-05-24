int main(int argc, char *argv[]);
__size32 add15(__size32 param1);
void add10();
void add5();
void printarg();

/** address: 0x00010b64 */
int main(int argc, char *argv[])
{
    add15(25);
    add10();
    add5();
    printarg();
    return 0;
}

/** address: 0x00010b40 */
__size32 add15(__size32 param1)
{
    return param1 + 15;
}

/** address: 0x00010b38 */
void add10()
{
    return;
}

/** address: 0x00010b30 */
void add5()
{
    return;
}

/** address: 0x00010b48 */
void printarg()
{
    printf(0x116e8);
    return;
}

