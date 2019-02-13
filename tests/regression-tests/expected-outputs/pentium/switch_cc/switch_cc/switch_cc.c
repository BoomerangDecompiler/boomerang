int main(int argc, char *argv[]);


/** address: 0x080488f0 */
int main(int argc, char *argv[])
{
    if ((unsigned int)(argc - 2) <= 5) {
        switch(argc) {
        case 2:
            printf("Two!\n");
            break;
        case 3:
            printf("Three!\n");
            break;
        case 4:
            printf("Four!\n");
            break;
        case 5:
            printf("Five!\n");
            break;
        case 6:
            printf("Six!\n");
            break;
        case 7:
            printf("Seven!\n");
            break;
        }
    }
    else {
        printf("Other!\n");
    }
    return 0;
}

