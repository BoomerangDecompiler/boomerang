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
bb0x10934:
            printf(0x10a38);
            goto bb0x10934;
        case 3:
            printf(0x10a40);
bb0x10954:
            goto bb0x10954;
        case 4:
            printf(0x10a48);
bb0x10968:
            goto bb0x10968;
        case 5:
            printf(0x10a50);
bb0x1097c:
            goto bb0x1097c;
        case 6:
            printf(0x10a58);
bb0x10990:
            goto bb0x10990;
        case 7:
            printf(0x10a60);
bb0x109a4:
            goto bb0x109a4;
        }
    }
    return 0;
}

