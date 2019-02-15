int main(int argc, char *argv[]);


/** address: 0x00010694 */
int main(int argc, char *argv[])
{
    union { int; float; } f8; 		// r40
    union { int; float; } local0; 		// m[o6 - 24]
    union { double; __size32; } local1; 		// m[o6 - 12]

    f8 = *0x10940;
    scanf("%f", &local0);
    printf("a is %f, b is %f\n", f8, local1);
    if (f8 == local0) {
        printf("Equal\n");
    }
    if (f8 != local0) {
        printf("Not Equal\n");
    }
    if (f8 > local0) {
        printf("Greater\n");
    }
    if (f8 <= local0) {
        printf("Less or Equal\n");
    }
    if (f8 >= local0) {
        printf("Greater or Equal\n");
    }
    if (f8 < local0) {
        printf("Less\n");
    }
    return 0;
}

