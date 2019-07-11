int main(int argc, char *argv[]);


/** address: 0x08048918 */
int main(int argc, char *argv[])
{
    char *local0; 		// m[esp - 8]

    if ((unsigned int)(argc - 2) > 5) {
        local0 = "Other!\n";
    }
    else {
        switch(argc) {
        case 2:
            local0 = "Two!\n";
            break;
        case 3:
            local0 = "Three!\n";
            break;
        case 4:
            local0 = "Four!\n";
            break;
        case 5:
            local0 = "Five!\n";
            break;
        case 6:
            local0 = "Six!\n";
            break;
        case 7:
            local0 = "Seven!\n";
            break;
        }
    }
    printf(local0);
    return 0;
}

