int main(int argc, char *argv[]);
__size32 atexit(atexitfunc param1);
void MAIN__(__size32 param1);


/** address: 0x08048824 */
int main(int argc, char *argv[])
{
    __size32 eax; 		// r24
    int ebp; 		// r29
    int ecx; 		// r25
    int edx; 		// r26
    int esp; 		// r28

    f_setarg(argc, argv);
    f_setsig();
    f_init();
    eax = atexit(0x8048584); /* Warning: also results in ecx, edx */
    MAIN__(eax, ecx, edx, esp - 4, SUBFLAGS32((esp - 12), 16, esp - 28), esp == 28, (unsigned int)(esp - 12) < 16, (int)esp < 28, (int)esp < 12 && (int)esp >= 28, argc, argv, ebp, argv, 0x8048584, pc);
}

/** address: 0x08048904 */
__size32 atexit(atexitfunc param1)
{
    void *eax; 		// r24
    int eax_1; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    eax = 0;
    if (edx != 0) {
        eax = *edx;
    }
    eax_1 = __cxa_atexit(param1, 0, eax); /* Warning: also results in ecx, edx */
    return eax_1; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x080486cc */
void MAIN__(__size32 param1)
{
    int local4; 		// m[esp - 16]

    s_wsle();
    do_lio(0x80489ac, 0x80489a8, 0x804897c, 10);
    e_wsle();
    s_rsle();
    do_lio(0x80489b0, 0x80489a8, &param1, 4);
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

