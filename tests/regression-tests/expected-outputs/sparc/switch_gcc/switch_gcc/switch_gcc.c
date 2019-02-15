int main(int argc, char *argv[]);


/** address: 0x00010a54 */
int main(int argc, char *argv[])
{
    int o0; 		// r8

    if ((unsigned int)(argc - 2) > 5) {
        o0 = "Other!\n";
    }
    else {
        switch(argc) {
        case 2:
            o0 = "Two!\n";
            break;
        case 3:
            o0 = "Three!\n";
            break;
        case 4:
            o0 = "Four!\n";
            break;
        case 5:
            o0 = "Five!\n";
            break;
        case 6:
            o0 = "Six!\n";
            break;
        case 7:
            o0 = "Seven!\n";
            break;
        }
    }
    printf(o0);
    return 0;
}

