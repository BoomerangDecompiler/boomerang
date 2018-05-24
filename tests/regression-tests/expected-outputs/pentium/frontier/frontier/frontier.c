int main(int argc, char *argv[]);

/** address: 0x080482f4 */
int main(int argc, char *argv[])
{
    int local0; 		// m[esp + 4]
    int local1; 		// m[esp + 4]{0}
    int local2; 		// m[esp + 4]{0}
    int local3; 		// local2{0}
    int local4; 		// local0{0}

    local3 = argc;
    if (argc == 5) {
        do {
            local2 = local3;
            local1 = local2 - 1;
            local4 = local1;
            if (local2 <= 1) {
                if (local1 == 12) {
bb0x80483b2:
                    return 13;
                }
bb0x8048386:
                local0 = local4;
                local3 = local0;
            }
            else {
                local0 = local1 - 1;
                local4 = local0;
                if (local1 > 2) {
bb0x8048347:
                    return 13;
                }
                goto bb0x8048386;
            }
            return 13;
        } while (local0 > 0);
    }
    else {
        if (argc > 5) {
            if (argc == 9) {
                if (argc != 10) {
                    goto bb0x80483b2;
                }
                else {
                    goto bb0x80483b2;
                }
                goto bb0x80483b2;
            }
            else {
            }
        }
        else {
            if (argc == 2) {
                do {
                } while (argc > 0);
                goto bb0x8048347;
            }
            else {
            }
        }
    }
    return 13;
}

