void atexit();
void MAIN__(__size32 param1);

// address: 8048824
int main(int argc, char *argv[], char *envp[]) {
    __size32 local0; 		// m[esp - 40]

    f_setarg();
    f_setsig();
    f_init();
    atexit();
    MAIN__(local0);
    exit(0);
    return;
}

// address: 8048904
void atexit() {
    __size32 edx; 		// r26

    if (edx != 0) {
    }
    __cxa_atexit();
    return;
}

// address: 80486cc
void MAIN__(__size32 param1) {
    int local0; 		// m[esp - 16]

    s_wsle();
    do_lio();
    e_wsle();
    s_rsle();
    do_lio();
    e_rsle();
    if (param1 == 2) {
    }
    if (param1 == 3) {
    }
    if (param1 == 4) {
    }
    switch(local0) {
    case 0x8048760:
        s_wsle();
        do_lio();
        e_wsle();
        break;
    case 0x8048793:
        s_wsle();
        do_lio();
        e_wsle();
        break;
    case 0x80487c3:
        s_wsle();
        do_lio();
        e_wsle();
        break;
    case 0x80487f3:
        s_wsle();
        do_lio();
        e_wsle();
        break;
    }
    return;
}

