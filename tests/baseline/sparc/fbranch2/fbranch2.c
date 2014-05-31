union { float x327; int x328; } global1;

// address: 10694
int main(int argc, char *argv[], char *envp[]) {
    float local0; 		// m[o6 - 24]
    double local1; 		// m[o6 - 12]

    scanf("%f", &local0);
    printf("a is %f, b is %f\n", global1, local1);
    if (global1 == local0) {
        printf("Equal\n");
    }
    if (global1 != local0) {
        printf("Not Equal\n");
    }
    if (global1 > local0) {
        printf("Greater\n");
    }
    if (global1 <= local0) {
        printf("Less or Equal\n");
    }
    if (global1 >= local0) {
        printf("Greater or Equal\n");
    }
    if (global1 < local0) {
        printf("Less\n");
    }
    return 0;
}

