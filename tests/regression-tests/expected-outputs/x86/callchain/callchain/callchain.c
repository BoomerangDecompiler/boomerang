int main(int argc, char *argv[]);
__size32 add15(__size32 param1);
__size32 add10(__size32 param1);
__size32 add5(__size32 param1);
void printarg(int param1);


/** address: 0x080489c4 */
int main(int argc, char *argv[])
{
    __size32 eax; 		// r24

    eax = add15(25);
    eax = add10(eax);
    eax = add5(eax);
    printarg(eax);
    return 0;
}

/** address: 0x080489a0 */
__size32 add15(__size32 param1)
{
    return param1 + 15;
}

/** address: 0x08048990 */
__size32 add10(__size32 param1)
{
    return param1 + 10;
}

/** address: 0x08048980 */
__size32 add5(__size32 param1)
{
    return param1 + 5;
}

/** address: 0x080489b0 */
void printarg(int param1)
{
    printf("Fifty five is %d\n", param1);
    return;
}

