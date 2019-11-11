int main(int argc, char *argv[]);


/** address: 0x08048390 */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    float local0; 		// m[esp - 8]
    long double st; 		// r32

    scanf("%f", &local0);
    eax = printf("a is %f, b is %f\n", 0, 2.3125);
    st = local0;
    eax = ((eax & ~0xff00 | (SETFFLAGS(st, 5.)) << 8) & ~0xff & ~0xff00 | (SETFFLAGS(st, 5.) & 0x45) << 8) & ~0xff00 | (SETFFLAGS(st, 5.) & 0x45 ^ 64) << 8;
    if (st == 5.) {
        eax = puts("Equal");
    }
    st = local0;
    eax = (eax & ~0xff00 | (SETFFLAGS(5., st)) << 8) & ~0xff & ~0xff00 | (SETFFLAGS(5., st) & 0x45) << 8;
    if (5. != st) {
        eax = puts("Not Equal");
    }
    st = local0;
    eax = (eax & ~0xff00 | (SETFFLAGS(5., st)) << 8) & ~0xff;
    if (5. > st) {
        eax = puts("Greater");
    }
    st = local0;
    eax = (eax & ~0xff00 | (SETFFLAGS(st, 5.)) << 8) & ~0xff;
    if (st >= 5.) {
        eax = puts("Less or Equal");
    }
    st = local0;
    eax = (eax & ~0xff00 | (SETFFLAGS(5., st)) << 8) & ~0xff;
    if (5. >= st) {
        eax = puts("Greater or Equal");
    }
    st = local0;
    eax = (eax & ~0xff00 | (SETFFLAGS(st, 5.)) << 8) & ~0xff;
    if (st > 5.) {
        eax = puts("Less");
    }
    return eax;
}

