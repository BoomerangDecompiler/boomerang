int main(int argc, char *argv[]);


/** address: 0x00001bb8 */
int main(int argc, char *argv[])
{
    void *g1; 		// r1
    float local0; 		// m[g1 - 44]
    union { double; __size32; } local1; 		// m[g1 - 28]

    scanf("%f", &local0);
    printf("a is %f, b is %f\n", 5., local1);
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
    return g1 - 44;
}

