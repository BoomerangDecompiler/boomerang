int main(int argc, char *argv[]);

/** address: 0x00001c90 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 + 24]
    int local1; 		// m[g1 + 24]{0}
    int local2; 		// m[g1 + 24]{0}
    int local3; 		// local0{0}

    local0 = argc;
    if (argc == 5) {
        do {
            local2 = local0;
            local1 = local2 - 1;
            local3 = local1;
            if (local2 <= 1) {
                if (local1 == 12) {
bb0x1db4:
                    return 13;
                }
bb0x1d74:
                local0 = local3;
            }
            else {
                local0 = local1 - 1;
                local3 = local0;
                if (local1 > 2) {
bb0x1d08:
                    return 13;
                }
                goto bb0x1d74;
            }
            return 13;
        } while (local0 > 0);
    }
    else {
        if (argc > 5) {
            if (argc == 9) {
                if (argc != 10) {
                    goto bb0x1db4;
                }
                else {
                    goto bb0x1db4;
                }
                goto bb0x1db4;
            }
            else {
            }
        }
        else {
            if (argc == 2) {
                do {
                } while (argc > 0);
                goto bb0x1d08;
            }
            else {
            }
        }
    }
    return 13;
}

