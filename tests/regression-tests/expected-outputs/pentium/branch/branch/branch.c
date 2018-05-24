int main(int argc, char *argv[]);

/** address: 0x08048948 */
int main(int argc, char *argv[])
{
    int local0; 		// m[esp - 8]
    unsigned int local1; 		// m[esp - 12]

    scanf("%d", &local0);
    scanf("%d", &local1);
    if (local0 == 5) {
        printf("Equal\n");
    }
    if (local0 == 5) {
        printf("Less or Equal\n");
        if (local0 > 5) {
            printf("Less\n");
        }
        else {
            printf("Greater or Equal\n");
bb0x80489cd:
            if (local0 > 5) {
                goto bb0x80489cd;
            }
        }
    }
    else {
        printf("Not Equal\n");
bb0x8048997:
        if (local0 >= 5) {
            goto bb0x8048997;
        }
        else {
            printf("Greater\n");
bb0x80489a9:
            if (local0 < 5) {
                goto bb0x80489a9;
            }
            else {
                goto bb0x80489a9;
            }
        }
    }
    if (local1 >= 5) {
        printf("Less or Equal Unsigned\n");
        if (local1 > 5) {
            printf("Carry Set\n");
        }
        else {
            printf("Carry Clear\n");
bb0x8048a15:
            if (local1 > 5) {
                goto bb0x8048a15;
            }
        }
    }
    else {
        printf("Greater Unsigned\n");
bb0x80489f1:
        if (local1 < 5) {
            goto bb0x80489f1;
        }
        else {
            goto bb0x80489f1;
        }
    }
    if (5 - local0 < 0) {
        printf("Plus\n");
    }
    else {
        printf("Minus\n");
bb0x8048a3b:
        if (5 - local0 < 0) {
            goto bb0x8048a3b;
        }
    }
    return 0;
}

