int main(int argc, char *argv[]);

/** address: 0x0001090c */
int main(int argc, char *argv[])
{
    if ((unsigned int)(argc - 2) > 5) {
        printf("Other!\n");
    }
    else {
        switch(argc) {
        case 2:
            printf("Two!\n");
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
    return 0;
}

