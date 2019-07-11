int main(int argc, char *argv[]);


/** address: 0x00001b78 */
int main(int argc, char *argv[])
{
    __size32 g3; 		// r3
    int local0; 		// m[g1 - 32]
    unsigned int local1; 		// m[g1 - 28]

    scanf("%d", &local0);
    scanf("%d", &local1);
    if (local0 == 5) {
        puts("Equal");
    }
    if (local0 != 5) {
        puts("Not Equal");
    }
    if (5 > local0) {
        puts("Greater");
    }
    if (5 <= local0) {
        puts("Less or Equal");
    }
    if (5 >= local0) {
        puts("Greater or Equal");
    }
    if (5 < local0) {
        puts("Less");
    }
    if (5 > local1) {
        puts("Greater Unsigned");
    }
    if (5 <= local1) {
        puts("Less or Equal Unsigned");
    }
    if (5 >= local1) {
        puts("Carry Clear");
    }
    if (5 < local1) {
        puts("Carry Set");
    }
    if (5 >= local0) {
        puts("Minus");
    }
    g3 = 5 - local0;
    if (5 < local0) {
        g3 = puts("Plus");
    }
    return g3;
}

