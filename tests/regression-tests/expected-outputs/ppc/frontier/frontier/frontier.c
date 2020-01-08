int main(int argc, char *argv[]);


/** address: 0x100003f0 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 40]
    int local1; 		// m[g1 - 40]{11}
    int local2; 		// m[g1 - 40]{10}
    int local3; 		// local0{15}

    local0 = argc;
    if (argc == 5) {
        do {
            local2 = local0;
            local1 = local2 - 1;
            local3 = local1;
            if (local2 <= 1) {
bb0x100004bc:
                if (local1 == 12) {
                    break;
                }
bb0x100004d4:
                local0 = local3;
            }
            else {
                local0 = local1 - 1;
                local3 = local0;
                if (local1 > 2) {
                    break;
                }
                goto bb0x100004d4;
            }
            goto bb0x100004bc;
        } while (local0 > 0);
    }
    else {
        if (argc > 5) {
            if (argc == 9) {
                if (argc != 10) {
                }
            }
        }
        else {
            if (argc == 2) {
            }
        }
    }
    return 13;
}

