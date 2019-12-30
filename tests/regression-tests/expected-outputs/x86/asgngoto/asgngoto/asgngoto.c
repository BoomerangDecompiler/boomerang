int main(int argc, char *argv[]);
void atexit(atexitfunc param1);
void MAIN__(char param1);


/** address: 0x08048824 */
int main(int argc, char *argv[])
{
    char local0; 		// m[esp - 40]

    f_setarg(argc, argv);
    f_setsig();
    f_init();
    atexit(0x8048584);
    MAIN__(local0);
    exit(0);
    return;
}

/** address: 0x08048904 */
void atexit(atexitfunc param1)
{
    void *eax; 		// r24
    int edx; 		// r26

    eax = 0;
    if (edx != 0) {
        eax = *edx;
    }
    __cxa_atexit(param1, 0, eax);
    return;
}

/** address: 0x080486cc */
void MAIN__(char param1)
{
    __size32 local0; 		// m[esp - 16]

    s_wsle(0x8049afc);
    do_lio(0x80489ac, 0x80489a8, "Input num:Input out of rangeTwo!Three!Four!", 10);
    e_wsle();
    s_rsle(0x8049b10);
    do_lio(0x80489b0, 0x80489a8, &param1, 4);
    e_rsle();
    local0 = 0x8048760;
    if (param1 == 2) {
        local0 = 0x8048793;
    }
    if (param1 == 3) {
        local0 = 0x80487c3;
    }
    if (param1 == 4) {
        local0 = 0x80487f3;
    }
    switch(local0) {
    case 0x8048760:
        s_wsle(0x8049b24);
        do_lio(0x80489ac, 0x80489a8, "Input out of rangeTwo!Three!Four!", 18);
        e_wsle();
        break;
    case 0x8048793:
        s_wsle(0x8049b38);
        do_lio(0x80489ac, 0x80489a8, "Two!Three!Four!", 4);
        e_wsle();
        break;
    case 0x80487c3:
        s_wsle(0x8049b4c);
        do_lio(0x80489ac, 0x80489a8, "Three!Four!", 6);
        e_wsle();
        break;
    case 0x80487f3:
        s_wsle(0x8049b60);
        do_lio(0x80489ac, 0x80489a8, "Four!", 5);
        e_wsle();
        break;
    }
    return;
}

