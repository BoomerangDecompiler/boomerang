int main(int argc, char *argv[]);


/** address: 0x00001c7c */
int main(int argc, char *argv[])
{
    if ((unsigned int)argc > 7) {
bb0x1d54:
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
    case 0:
    case 1:
        goto bb0x1d54;
    }
    return 0;
}

