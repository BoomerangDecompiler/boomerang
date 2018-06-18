int main(int argc, char *argv[]);
__size32 atexit();
void MAIN__(__size32 param1);

/** address: 0x08048824 */
int main(int argc, char *argv[])
{
    __size32 eax; 		// r24
    __size32 ebp; 		// r29
    int ecx; 		// r25
    int edx; 		// r26
    int esp; 		// r28

    f_setarg();
    f_setsig();
    f_init();
    eax = atexit(); /* Warning: also results in ecx, edx */
    MAIN__(pc, 0x8048584, argv, ebp, argc, argv, eax, ecx, edx, esp - 4, SUBFLAGS32((esp - 12), 16, esp - 28), esp == 28, esp - 12 < (unsigned int)16);
}

/** address: 0x08048904 */
__size32 atexit()
{
    __size32 eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    if (edx != 0) {
    }
    eax = __cxa_atexit(); /* Warning: also results in ecx, edx */
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x080486cc */
void MAIN__(__size32 param1)
{
    int local0; 		// m[esp - 16]

    s_wsle();
    do_lio();
    e_wsle();
    s_rsle();
    do_lio();
    e_rsle();
    if (param1 != 2) {
bb0x8048741:
        if (param1 != 3) {
bb0x804874e:
            if (param1 != 4) {
bb0x804875b:
            }
            goto bb0x804875b;
        }
        goto bb0x804874e;
    }
    goto bb0x8048741;
}

