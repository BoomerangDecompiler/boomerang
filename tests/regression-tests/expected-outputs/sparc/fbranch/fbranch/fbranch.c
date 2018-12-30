int global_0x00010938 = 0x40a00000;
int main(int argc, char *argv[]);

/** address: 0x000106d0 */
int main(int argc, char *argv[])
{
    union { __size32; float; } f3; 		// r35
    union { __size32; float; } local0; 		// m[o6 - 20]
    __size32 local1; 		// m[o6 - 12]

    scanf("%f", &local0);
    f3 = *0x10938;
    printf("a is %f, b is %f\n", f3, local1);
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

