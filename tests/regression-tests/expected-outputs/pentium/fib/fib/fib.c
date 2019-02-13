int main(int argc, char *argv[]);
__size32 fib(int param1);


/** address: 0x080483cf */
int main(int argc, char *argv[])
{
    int eax; 		// r24

    eax = fib(10);
    printf("%i\n", eax);
    return 0;
}

/** address: 0x08048390 */
__size32 fib(int param1)
{
    int eax; 		// r24
    int eax_1; 		// r24{4}
    int local4; 		// m[esp - 12]

    if (param1 <= 1) {
        local4 = param1;
    }
    else {
        eax_1 = fib(param1 - 1);
        eax = fib(param1 - 2);
        local4 = eax_1 + eax;
    }
    return local4;
}

