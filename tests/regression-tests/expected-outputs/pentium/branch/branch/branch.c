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
bb0x80489b1:
        printf("Less or Equal\n");
        if (local0 > 5) {
bb0x80489d5:
            printf("Less\n");
        }
        else {
bb0x80489c3:
            printf("Greater or Equal\n");
            if (local0 > 5) {
                goto bb0x80489d5;
            }
        }
    }
    else {
        printf("Not Equal\n");
        if (local0 >= 5) {
            goto bb0x80489b1;
        }
        else {
            printf("Greater\n");
            if (local0 < 5) {
                goto bb0x80489c3;
            }
            else {
                goto bb0x80489b1;
            }
        }
    }
    if (local1 >= 5) {
bb0x80489f9:
        printf("Less or Equal Unsigned\n");
        if (local1 > 5) {
bb0x8048a1d:
            printf("Carry Set\n");
        }
        else {
bb0x8048a0b:
            printf("Carry Clear\n");
            if (local1 > 5) {
                goto bb0x8048a1d;
            }
        }
    }
    else {
        printf("Greater Unsigned\n");
        if (local1 < 5) {
            goto bb0x8048a0b;
        }
        else {
            goto bb0x80489f9;
        }
    }
    if (5 < local0) {
bb0x8048a45:
        printf("Plus\n");
    }
    else {
        printf("Minus\n");
        if (5 < local0) {
            goto bb0x8048a45;
        }
    }
    return 0;
}

