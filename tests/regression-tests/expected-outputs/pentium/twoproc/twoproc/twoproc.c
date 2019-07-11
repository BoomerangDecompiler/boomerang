int main(int argc, char *argv[]);
__size32 proc1(__size32 param1, __size32 param2);


/** address: 0x08048375 */
int main(int argc, char *argv[])
{
    int eax; 		// r24

    eax = proc1(11, 4);
    eax = printf("%i\n", eax);
    return eax;
}

/** address: 0x08048368 */
__size32 proc1(__size32 param1, __size32 param2)
{
    return param1 - param2;
}

