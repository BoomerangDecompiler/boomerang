int main(unsigned int argc, char *argv[]);

/** address: 0x000106a4 */
int main(unsigned int argc, char *argv[])
{
    char *local0; 		// o0_7{19}
    int o0; 		// r8
    char *o0_1; 		// r8{4}
    char *o0_2; 		// r8{8}
    char *o0_3; 		// r8{10}
    char *o0_4; 		// r8{6}
    char *o0_5; 		// r8{14}
    char *o0_6; 		// r8{12}
    char *o0_7; 		// r8{19}

    if (argc > 7) {
bb0x106d0:
        o0_1 = "Other!";
        local0 = o0_1;
        break;
    }
    switch(argc) {
    case 0:
        goto bb0x106d0;
    case 1:
        goto bb0x106d0;
    case 2:
bb0x106e8:
        o0_2 = "Two!";
        local0 = o0_2;
        break;
    case 3:
bb0x106f4:
        o0_3 = "Three!";
        local0 = o0_3;
        break;
    case 4:
        if (7 - argc <= 5) {
            switch(7 - argc) {
            case 0:
bb0x106dc:
                o0_4 = "Seven!";
                local0 = o0_4;
                break;
            case 1:
bb0x1070c:
                o0_5 = "Six!";
                local0 = o0_5;
                break;
            case 2:
bb0x10700:
                o0_6 = "Five!";
                local0 = o0_6;
                break;
            case 3:
                o0 = "Four!";
                local0 = o0;
                break;
            case 4:
                goto bb0x106f4;
            case 5:
                goto bb0x106e8;
            }
            goto bb0x10744;
        }
        goto bb0x106d0;
    case 5:
        goto bb0x10700;
    case 6:
        goto bb0x1070c;
    case 7:
        goto bb0x106dc;
    }
bb0x10744:
    o0_7 = local0;
    puts(o0_7);
    return 0;
}

