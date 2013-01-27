// address: 0x10a80
int main(int argc, char *argv[], char *envp[]) {
    int local0; 		// m[o6 - 20]
    unsigned int local1; 		// m[o6 - 24]

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
    printf("Less\n");
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
    return 0;
}

