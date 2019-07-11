int main(int argc, char *argv[]);

union { int; float; } global_0x00010938;

/** address: 0x000106d0 */
int main(int argc, char *argv[])
{
    union { int; float; } local0; 		// m[o6 - 20]
    union { double; __size32; } local1; 		// m[o6 - 12]

    scanf("%f", &local0);
    printf("a is %f, b is %f\n", global_0x00010938, local1);
    if (global_0x00010938 != local0) {
bb0x10768:
        puts("Not Equal");
    }
    else {
        puts("Equal");
        if (global_0x00010938 != local0) {
            goto bb0x10768;
        }
    }
    if (global_0x00010938 > local0) {
        puts("Greater");
    }
    if (global_0x00010938 <= local0) {
        puts("Less or Equal");
    }
    if (global_0x00010938 >= local0) {
        puts("Greater or Equal");
    }
    if (global_0x00010938 < local0) {
        puts("Less");
    }
    return 0x10800;
}

