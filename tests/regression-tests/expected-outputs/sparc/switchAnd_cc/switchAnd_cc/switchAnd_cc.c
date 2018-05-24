int main(int argc, char *argv[]);

/** address: 0x0001060c */
int main(int argc, char *argv[])
{
    if (argc >= 2) {
        switch(argc - 2 & 0x7) {
        case 0:
            printf(0x10774);
            return 0;
        case 1:
            printf(0x1077c);
            return 0;
        }
    }
    else {
        printf(0x1076c);
    }
    return 0;
}

