int main(int argc, char *argv[]);


/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    void *g1; 		// r1
    double g4_1; 		// r4
    int g5; 		// r5
    float local0; 		// m[g1 - 20]

    g4_1 = scanf("%f", &local0); /* Warning: also results in g5 */
    printf("a is %f, b is %f\n", g4_1, g5);
    if (0x40a00000 == local0) {
        printf("Equal\n");
    }
    if (0x40a00000 != local0) {
        printf("Not Equal\n");
    }
    if (5. > local0) {
        printf("Greater\n");
    }
    if (0x40a00000 == local0) {
        printf("Less or Equal\n");
    }
    if (0x40a00000 == local0) {
        printf("Greater or Equal\n");
    }
    if (5. < local0) {
        printf("Less\n");
    }
    return g1 - 20;
}

