int main(int argc, char *argv[]);

/** address: 0x0001090c */
int main(int argc, char *argv[])
{
    if ((unsigned int)(argc - 2) > (unsigned int)5) {
        printf(0x10a68);
    }
    else {
        switch(argc) {
        case 2:
            printf(0x10a38);
            return 0;
        case 3:
            printf(0x10a40);
            return 0;
        case 4:
            printf(0x10a48);
            return 0;
        case 5:
            printf(0x10a50);
            return 0;
        case 6:
            printf(0x10a58);
            return 0;
        case 7:
            printf(0x10a60);
            return 0;
        }
    }
    return 0;
}

