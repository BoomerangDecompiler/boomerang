int main(int argc, char *argv[]);

/** address: 0x080488f0 */
int main(int argc, char *argv[])
{
    if ((unsigned int)(argc - 2) <= 5) {
        switch(argc) {
        case 2:
            printf("Two!\n");
bb0x804890c:
            return 0;
        case 3:
            printf("Three!\n");
            return 0;
        case 4:
            printf("Four!\n");
            return 0;
        case 5:
            printf("Five!\n");
            return 0;
        case 6:
            printf("Six!\n");
            return 0;
        case 7:
            printf("Seven!\n");
            return 0;
        }
    }
    else {
        printf("Other!\n");
        goto bb0x804890c;
    }
    return 0;
}

