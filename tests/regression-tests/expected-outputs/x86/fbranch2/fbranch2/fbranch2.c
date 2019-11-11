int main(int argc, char *argv[]);


/** address: 0x080483e4 */
int main(int argc, char *argv[])
{
    float local0; 		// m[esp - 12]
    double local1; 		// m[esp - 68]

    scanf("%f", &local0);
    printf("a is %f, b is %f\n", 5., local1);
    if (0x40a00000 == local0) {
        puts("Equal");
    }
    if (0x40a00000 != local0) {
        puts("Not Equal");
    }
    if (5. > local0) {
        puts("Greater");
    }
    if (local0 >= 5.) {
        puts("Less or Equal");
    }
    if (5. >= local0) {
        puts("Greater or Equal");
    }
    if (local0 > 5.) {
        puts("Less");
    }
    return 0;
}

