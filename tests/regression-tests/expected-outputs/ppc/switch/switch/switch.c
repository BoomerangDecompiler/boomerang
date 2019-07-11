int main(int argc, char *argv[]);


/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    if ((unsigned int)argc > 7) {
bb0x100004c8:
        printf("Other!\n");
        break;
    }
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
    case 1:
    case 0:
        goto bb0x100004c8;
    }
    return 0;
}

