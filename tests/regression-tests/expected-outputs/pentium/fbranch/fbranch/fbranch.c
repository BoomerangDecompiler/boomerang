int main(int argc, char *argv[]);

/** address: 0x08048390 */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    float local0; 		// m[esp - 8]
    int st1; 		// r33
    double st7; 		// r39

    scanf("%f", &local0);
    eax = printf("a is %f, b is %f\n", 0, 2.3125);
    st7 = local0;
    eax = ((eax & ~0xff00 | SETFFLAGS(st7, st1) * 256) >> 8 & 0xffffff & ~0xff00 | (SETFFLAGS(st7, st1) & 0x45) * 256) & ~0xff00 | (SETFFLAGS(st7, st1) & 0x45 ^ 64) * 256;
    if (st7 == st1) {
        eax = puts("Equal");
    }
    st7 = local0;
    eax = (eax & ~0xff00 | SETFFLAGS(5., st7) * 256) >> 8 & 0xffffff & ~0xff00 | (SETFFLAGS(5., st7) & 0x45) * 256;
    if (5. != st7) {
        eax = puts("Not Equal");
    }
    st7 = local0;
    eax = (eax & ~0xff00 | SETFFLAGS(5., st7) * 256) >> 8 & 0xffffff;
    if (5. > st7) {
        eax = puts("Greater");
    }
    st7 = local0;
    eax = (eax & ~0xff00 | SETFFLAGS(st7, st1) * 256) >> 8 & 0xffffff;
    if (st7 >= st1) {
        eax = puts("Less or Equal");
    }
    st7 = local0;
    eax = (eax & ~0xff00 | SETFFLAGS(5., st7) * 256) >> 8 & 0xffffff;
    if (5. >= st7) {
        eax = puts("Greater or Equal");
    }
    st7 = local0;
    eax = (eax & ~0xff00 | SETFFLAGS(st7, 5.) * 256) >> 8 & 0xffffff;
    if (st7 > 5.) {
        eax = puts("Less");
    }
    return eax;
}

