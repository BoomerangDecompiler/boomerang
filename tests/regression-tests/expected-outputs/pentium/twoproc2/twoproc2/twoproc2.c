int main(int argc, char *argv[]);
__size32 proc1(__size32 param1, __size32 param2);


/** address: 0x08048333 */
int main(int argc, char *argv[])
{
    int eax; 		// r24

    eax = proc1(3, 4);
    printf("%i\n", eax);
    eax = proc1(5, 6);
    eax = printf("%i\n", eax);
    return eax;
}

/** address: 0x08048328 */
__size32 proc1(__size32 param1, __size32 param2)
{
    __size32 eax; 		// r24

    eax = param2 + param1;
    return param2 + param1;
}

