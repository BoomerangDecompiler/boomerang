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
                    goto bb0x1dbc;
                }
                local0 = local3;
            }
            else {
bb0x1d38:
                local0 = local1 - 1;
                local3 = local0;
                if (local1 > 2) {
                    goto bb0x1dbc;
                }
                goto bb0x1d38;
            }
            goto bb0x1dbc;
        } while (local0 > 0);
    }
    else {
        if (argc > 5) {
            if (argc == 9) {
bb0x1d8c:
                if (argc != 10) {
bb0x1dac:
                    goto bb0x1dac;
                }
                else {
bb0x1da0:
                    goto bb0x1da0;
                }
                goto bb0x1d8c;
            }
            else {
            }
        }
        else {
            if (argc == 2) {
                do {
bb0x1cf4:
                } while (argc > 0);
                goto bb0x1cf4;
            }
            else {
            }
        }
    }
bb0x1dbc:
    return 13;
}

