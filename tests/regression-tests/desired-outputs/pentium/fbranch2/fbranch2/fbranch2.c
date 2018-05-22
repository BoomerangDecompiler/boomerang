int main(int argc, char *argv[]);

/** address: 0x080483e4 */
int main(int argc, char *argv[])
{
    float local0; 		// m[esp - 12]
    double local1; 		// m[esp - 68]
    double st7; 		// r39

    scanf("%f", &local0);
    printf("a is %f, b is %f\n", 5., local1);
    if (0x40a00000 == local0) {
        puts("Equal");
    }
    else {
    }
    if (0x40a00000 != local0) {
        puts("Not Equal");
    }
    else {
    }
    if (5. > local0) {
        puts("Greater");
    }
    else {
    }
    if (local0 >= 5.) {
        puts("Less or Equal");
    }
    else {
    }
    if (5. >= local0) {
        puts("Greater or Equal");
    }
    else {
    }
    st7 = local0;
    if (local0 > 5.) {
        puts("Less");
    }
    else {
    }
    return 0;
}

