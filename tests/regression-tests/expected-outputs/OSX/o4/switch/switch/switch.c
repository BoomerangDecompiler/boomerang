int main(int argc, char *argv[]);


/** address: 0x00001cb4 */
int main(int argc, char *argv[])
{
    int g3; 		// r3

    if ((unsigned int)argc > 7) {
bb0x1d58:
        g3 = "Other!";
        break;
    }
    switch(argc) {
    case 2:
        g3 = "Two!";
        break;
    case 3:
        g3 = "Three!";
        break;
    case 4:
        g3 = "Four!";
        break;
    case 5:
        g3 = "Five!";
        break;
    case 6:
        g3 = "Six!";
        break;
    case 7:
        g3 = "Seven!";
        break;
    case 0:
    case 1:
        goto bb0x1d58;
    }
    puts(g3);
    return 0;
}

