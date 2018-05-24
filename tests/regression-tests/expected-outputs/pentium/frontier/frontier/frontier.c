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
                    goto bb0x80483b9;
                }
                local0 = local4;
                local3 = local0;
            }
            else {
bb0x8048363:
                local0 = local1 - 1;
                local4 = local0;
                if (local1 > 2) {
                    goto bb0x80483b9;
                }
                goto bb0x8048363;
            }
            goto bb0x80483b9;
        } while (local0 > 0);
    }
    else {
        if (argc > 5) {
            if (argc == 9) {
bb0x8048395:
                if (argc != 10) {
bb0x80483ab:
                    goto bb0x80483ab;
                }
                else {
bb0x80483a2:
                    goto bb0x80483a2;
                }
                goto bb0x8048395;
            }
            else {
            }
        }
        else {
            if (argc == 2) {
                do {
bb0x804833a:
                } while (argc > 0);
                goto bb0x804833a;
            }
            else {
            }
        }
    }
bb0x80483b9:
    return 13;
}

