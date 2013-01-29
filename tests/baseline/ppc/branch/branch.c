// address: 10000440
int main(int argc, char *argv[], char *envp[]) {
    int local0; 		// m[g1 - 36]
    unsigned int local1; 		// m[g1 - 28]

    scanf("%d", &local0);
    scanf("%d", &local1);
    if (5 == local0) {
        printf("Equal\n");
    }
    if (5 != local0) {
        printf("Not Equal\n");
    }
    if (5 > local0) {
        printf("Greater\n");
    }
    if (5 <= local0) {
        printf("Less or Equal\n");
    }
    if (5 >= local0) {
        printf("Greater or Equal\n");
    }
    if (5 < local0) {
        printf("Less\n");
    }
    if (5 > local1) {
        printf("Greater Unsigned\n");
    }
    if (5 <= local1) {
        printf("Less or Equal Unsigned\n");
    }
    if (5 >= local1) {
        printf("Carry Clear\n");
    }
    if (5 < local1) {
        printf("Carry Set\n");
    }
    if (5 - local0 >= 0) {
        printf("Minus\n");
    }
    if (5 - local0 < 0) {
        printf("Plus\n");
    }
    return 5 - local0;
}

