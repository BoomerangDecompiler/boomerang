int main(int argc, char *argv[]);


/** address: 0x080482f4 */
int main(int argc, char *argv[])
{
    int local0; 		// m[esp + 4]
    int local1; 		// m[esp + 4]{14}
    int local2; 		// m[esp + 4]{4}
    int local3; 		// argc{1}
    int local4; 		// argc{2}
    int local5; 		// argc{9}
    int local6; 		// local2{13}
    int local7; 		// local0{18}

    local6 = argc;
    local5 = argc;
    local5 = argc;
    local4 = argc;
    local4 = argc;
    local3 = argc;
    if (argc == 5) {
        do {
            local2 = local6;
            local1 = local2 - 1;
            local7 = local1;
            local4 = local1;
            if (local2 <= 1) {
bb0x8048377:
                if (local1 == 12) {
bb0x80483b2:
                    argc = local4;
                    local5 = argc;
                    argc = local5;
                    return 13;
                }
bb0x8048386:
                local0 = local7;
                local6 = local0;
                local5 = local0;
            }
            else {
                local0 = local1 - 1;
                local7 = local0;
                local3 = local0;
                if (local1 > 2) {
bb0x8048347:
                    argc = local3;
                    local5 = argc;
                    argc = local5;
                    return 13;
                }
                goto bb0x8048386;
            }
            goto bb0x8048377;
        } while (local0 > 0);
    }
    else {
        if (argc > 5) {
            if (argc == 9) {
                if (argc != 10) {
                    goto bb0x80483b2;
                }
                goto bb0x80483b2;
            }
        }
        else {
            if (argc == 2) {
                goto bb0x8048347;
            }
        }
    }
    argc = local5;
    return 13;
}

