int main(int argc, char *argv[]);

/** address: 0x08049190 */
int main(int argc, char *argv[])
{
    float local0; 		// m[esp - 4]
    float local1; 		// m[esp - 8]
    double local2; 		// m[esp - 20]

    scanf("%f", &local0);
    scanf("%f", &local1);
    printf("a is %f, b is %f\n", local0, local2);
    if (local0 == local1) {
        puts("Equal");
    }
    if (local0 != local1) {
        puts("Not Equal");
    }
    if (local0 > local1) {
        puts("Greater");
    }
    if (local1 >= local0) {
        puts("Less or Equal");
    }
    if (local0 >= local1) {
        puts("Greater or Equal");
    }
    if (local1 > local0) {
        puts("Less");
    }
    return 0;
}

