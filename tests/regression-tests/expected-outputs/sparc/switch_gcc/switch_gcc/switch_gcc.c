int main(int argc, char *argv[]);

/** address: 0x00010a54 */
int main(int argc, char *argv[])
{
    char *local0; 		// o0_1{17}
    int o0; 		// r8
    char *o0_1; 		// r8{17}
    char *o0_2; 		// r8{4}
    char *o0_3; 		// r8{6}
    char *o0_4; 		// r8{8}
    char *o0_5; 		// r8{10}
    char *o0_6; 		// r8{12}
    char *o0_7; 		// r8{14}

    if ((unsigned int)(argc - 2) > 5) {
        o0 = "Other!\n";
        local0 = o0;
    }
    else {
        switch(argc) {
        case 2:
            o0_2 = "Two!\n";
            local0 = o0_2;
            break;
        case 3:
            o0_3 = "Three!\n";
            local0 = o0_3;
            break;
        case 4:
            o0_4 = "Four!\n";
            local0 = o0_4;
            break;
        case 5:
            o0_5 = "Five!\n";
            local0 = o0_5;
            break;
        case 6:
            o0_6 = "Six!\n";
            local0 = o0_6;
            break;
        case 7:
            o0_7 = "Seven!\n";
            local0 = o0_7;
            break;
        }
    }
    o0_1 = local0;
    printf(o0_1);
    return 0;
}

