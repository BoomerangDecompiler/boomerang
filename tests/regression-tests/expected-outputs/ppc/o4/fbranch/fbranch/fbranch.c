int main(int argc, char *argv[]);


/** address: 0x10000468 */
int main(int argc, char *argv[])
{
    double f0; 		// r32
    __size32 g3; 		// r3
    float local0; 		// m[g1 - 24]

    scanf("%f", &local0);
    printf("a is %f, b is %f\n", 2.5250868e-29, 2.5243549e-29);
    if (5. == local0) {
        g3 = puts("Equal");
        if (5. != local0) {
bb0x100004c8:
            g3 = puts("Not Equal");
        }
    }
    else {
        goto bb0x100004c8;
    }
    if (5. > local0) {
        g3 = puts("Greater");
    }
    f0 = local0;
    if (5. == f0) {
        g3 = puts("Less or Equal");
        if (5. == local0) {
bb0x10000568:
            g3 = puts("Greater or Equal");
        }
    }
    else {
        if (5. == f0) {
            goto bb0x10000568;
        }
    }
    if (5. < local0) {
        g3 = puts("Less");
    }
    return g3;
}

