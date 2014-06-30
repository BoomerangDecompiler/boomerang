union { float x161; int x162; } global3;

// address: 0x106d0
int main(int argc, char *argv[], char *envp[]) {
    float local0; 		// m[o6 - 20]
    double local1; 		// m[o6 - 12]

    scanf("%f", &local0);
    printf("a is %f, b is %f\n", global3, local1);
    if (global3 != local0) {
L14:
        puts("Not Equal");
    } else {
        puts("Equal");
        if (global3 != local0) {
            goto L14;
        }
    }
    if (global3 > local0) {
        puts("Greater");
    } else {
    }
    if (global3 <= local0) {
        puts("Less or Equal");
    }
    if (global3 >= local0) {
        puts("Greater or Equal");
    }
    if (global3 < local0) {
        puts("Less");
    }
    return 0x10800;
}

