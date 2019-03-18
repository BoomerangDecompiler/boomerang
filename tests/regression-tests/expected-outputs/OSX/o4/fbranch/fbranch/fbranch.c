int main(int argc, char *argv[]);


/** address: 0x00001be4 */
int main(int argc, char *argv[])
{
    __size32 g3; 		// r3
    float local0; 		// m[g1 - 48]

    scanf("%f", &local0);
    g3 = printf("a is %f, b is %f\n", 2.3125, 0.);
    if (5. == local0) {
        g3 = puts("Equal");
    }
    if (5. != local0) {
        g3 = puts("Not Equal");
    }
    if (5. > local0) {
        g3 = puts("Greater");
    }
    if (5. == local0) {
        g3 = puts("Less or Equal");
    }
    if (5. == local0) {
        g3 = puts("Greater or Equal");
    }
    if (5. < local0) {
        g3 = puts("Less");
    }
    return g3;
}

