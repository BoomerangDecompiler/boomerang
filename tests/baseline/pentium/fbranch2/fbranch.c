// address: 80483e4
int main(int argc, char *argv[], char *envp[]) {
    float local0; 		// m[esp - 12]
    double local1; 		// m[esp - 68]
    double st; 		// r32

    scanf("%f", &local0);
    printf("a is %f, b is %f\n", 5., local1);
    st = local0;
    if (0x40a00000 == st) {
        puts("Equal");
    }
    st = local0;
    if (0x40a00000 != st) {
        puts("Not Equal");
    }
    st = local0;
    if (5. > st) {
        puts("Greater");
    }
    st = local0;
    if (st >= 5.) {
        puts("Less or Equal");
    }
    st = local0;
    if (5. >= st) {
        puts("Greater or Equal");
    }
    st = local0;
    if (st > 5.) {
        puts("Less");
    }
    return 0;
}

