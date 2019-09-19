int main(int argc, char *argv[]);


/** address: 0x10000414 */
int main(int argc, char *argv[])
{
    if ((unsigned int)argc > 7) {
bb0x10000444:
        puts("Other!");
        break;
    }
    switch(argc) {
    case 0:
    case 1:
        goto bb0x10000444;
    case 7:
        puts("Seven!");
        break;
    case 6:
        puts("Six!");
        break;
    case 5:
        puts("Five!");
        break;
    case 4:
        puts("Four!");
        break;
    case 3:
        puts("Three!");
        break;
    case 2:
        puts("Two!");
        break;
    }
    return 0;
}

