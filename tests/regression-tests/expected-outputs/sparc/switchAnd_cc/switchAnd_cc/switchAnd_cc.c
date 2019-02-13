int main(int argc, char *argv[]);


/** address: 0x0001060c */
int main(int argc, char *argv[])
{
    if (argc >= 2) {
        switch(argc - 2 & 0x7) {
        case 0:
            printf("Two!\n");
            break;
        case 1:
            printf("Three!\n");
            break;
        }
    }
    else {
        printf("Other!\n");
    }
    return 0;
}

