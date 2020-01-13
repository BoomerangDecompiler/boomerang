int main(int argc, char *argv[]);


/** address: 0x1000043c */
int main(int argc, char *argv[])
{
    __size32 g3; 		// r3
    int local0; 		// m[g1 - 24]
    unsigned int local1; 		// m[g1 - 20]

    scanf("%d", &local0);
    scanf("%d", &local1);
    if (local0 == 5) {
        g3 = puts("Equal");
        if (local0 != 5) {
            g3 = puts("Not Equal");
        }
        if (5 > local0) {
bb0x100005b4:
            g3 = puts("Greater");
        }
        if (5 <= local0) {
bb0x10000498:
            g3 = puts("Less or Equal");
        }
    }
    else {
        puts("Not Equal");
        if (5 > local0) {
            goto bb0x100005b4;
        }
        else {
            goto bb0x10000498;
        }
    }
    if (5 >= local0) {
        g3 = puts("Greater or Equal");
    }
    if (5 < local0) {
        g3 = puts("Less");
    }
    if (5 > local1) {
        g3 = puts("Greater Unsigned");
    }
    if (5 <= local1) {
        g3 = puts("Less or Equal Unsigned");
    }
    if (5 >= local1) {
        g3 = puts("Carry Clear");
    }
    if (5 < local1) {
        g3 = puts("Carry Set");
    }
    if (5 >= local0) {
        g3 = puts("Minus");
    }
    if (5 < local0) {
        g3 = puts("Plus");
    }
    return g3;
}

