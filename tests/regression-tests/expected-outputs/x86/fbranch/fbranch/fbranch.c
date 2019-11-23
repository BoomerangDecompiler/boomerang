int main(int argc, char *argv[]);


/** address: 0x08049230 */
int main(int argc, char *argv[])
{
    float local0; 		// m[esp - 16]

    scanf("%f", &local0);
    printf("a is %f, b is %f\n", 0., 2.3125);
    if (5. == local0) {
        printf("Equal\n");
    }
    if (5. != local0) {
        printf("Not Equal\n");
    }
    if (5. > local0) {
        printf("Greater\n");
    }
    if (5. <= local0) {
        printf("Less or Equal\n");
    }
    if (5. >= local0) {
        printf("Greater or Equal\n");
    }
    if (5. < local0) {
        printf("Less\n");
    }
    return 0;
}

