int main(int argc, char *argv[]);
__size32 test(int param1, int param2, int param3, __size32 param4);

/** address: 0x08048398 */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    __size32 ebp; 		// r29
    int esp; 		// r28
    int local3; 		// m[esp - 4]

    eax = test(4, 5, 6, (esp - 4)); /* Warning: also results in esp, ebp */
    *(__size32*)esp = eax;
    local3 = 0x80484b0;
    printf(*(esp - 4));
    *(__size32*)(esp - 8) = 4;
    *(__size32*)(esp - 12) = 5;
    *(__size32*)(esp - 16) = 6;
    eax = test(*(esp - 16), *(esp - 12), *(esp - 8), ebp); /* Warning: also results in esp, ebp */
    *(__size32*)(esp + 16) = eax;
    *(__size32*)(esp + 12) = 0x80484c8;
    printf(*(esp + 12));
    *(__size32*)(esp + 8) = 5;
    *(__size32*)(esp + 4) = 6;
    *(__size32*)esp = 4;
    eax = test(*esp, *(esp + 4), *(esp + 8), ebp); /* Warning: also results in esp, ebp */
    *(__size32*)(esp + 16) = eax;
    *(__size32*)(esp + 12) = 0x80484e0;
    printf(*(esp + 12));
    *(__size32*)(esp + 8) = 5;
    *(__size32*)(esp + 4) = 4;
    *(__size32*)esp = 6;
    eax = test(*esp, *(esp + 4), *(esp + 8), ebp); /* Warning: also results in esp */
    *(__size32*)(esp + 16) = eax;
    *(__size32*)(esp + 12) = 0x80484f8;
    printf(*(esp + 12));
    return 0;
}

/** address: 0x0804837c */
__size32 test(int param1, int param2, int param3, __size32 param4)
{
    __size32 ebp; 		// r29
    __size32 esp; 		// r28

    ebp = (esp - 4);
    if (param1 >= param2 || param2 >= param3) {
        ebp = param4;
    }
    ebp = *ebp;
    return 1; /* WARNING: Also returning: ebp := ebp */
}

