int main(int argc, char *argv[]);

/** address: 0x080488f0 */
int main(int argc, char *argv[])
{
    if ((unsigned int)(argc - 2) <= 5) {
        switch(argc) {
        case 2:
            printf("Two!\n");
bb0x804890c:
            goto bb0x804890c;
        case 3:
            printf("Three!\n");
bb0x8048932:
            goto bb0x8048932;
        case 4:
            printf("Four!\n");
bb0x8048946:
            goto bb0x8048946;
        case 5:
            printf("Five!\n");
bb0x804895a:
            goto bb0x804895a;
        case 6:
            printf("Six!\n");
bb0x804896e:
            goto bb0x804896e;
        case 7:
            printf("Seven!\n");
bb0x8048989:
            goto bb0x8048989;
        }
    }
    else {
bb0x8048902:
        printf("Other!\n");
        goto bb0x8048902;
    }
    return 0;
}

