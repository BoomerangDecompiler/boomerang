// address: 0x8048390
int main(int argc, char *argv[], char *envp[]) {
    __size32 eax; 		// r24
    float local0; 		// m[esp - 8]
    double st; 		// r32

    proc1();
    proc2();
    st = local0;
    if (st == 5.) {
        proc3();
        proc4();
        return;
    }
    if (5. != st) {
        proc3();
        proc5();
        return;
    }
    if (5. > st) {
        proc3();
        proc6();
        return;
    }
    eax = (((((((eax & 0xffff00ff | SETFFLAGS(st, 5.) * 256) >> 8 & 0xffffff & 0xffff00ff | (SETFFLAGS(st, 5.) & 0x45) * 256) & 0xffff00ff | (SETFFLAGS(st, 5.) & 0x45 ^ 64) * 256) & 0xffff00ff | SETFFLAGS(5., st) * 256) >> 8 & 0xffffff & 0xffff00ff | (SETFFLAGS(5., st) & 0x45) * 256) & 0xffff00ff | SETFFLAGS(5., st) * 256) >> 8 & 0xffffff & 0xffff00ff | SETFFLAGS(st, 5.) * 256) >> 8 & 0xffffff;
    if (st >= 5.) {
        proc3();
    }
    st = local0;
    eax = (eax & 0xffff00ff | SETFFLAGS(5., st) * 256) >> 8 & 0xffffff;
    if (5. >= st) {
        proc3();
    }
    st = local0;
    eax = (eax & 0xffff00ff | SETFFLAGS(st, 5.) * 256) >> 8 & 0xffffff;
    if (st > 5.) {
        proc3();
    }
    return eax;
}

