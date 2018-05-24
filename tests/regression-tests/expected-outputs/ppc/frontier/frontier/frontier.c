int main(int argc, char *argv[]);

/** address: 0x100003f0 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 40]
    int local1; 		// m[g1 - 40]{0}
    int local2; 		// m[g1 - 40]{0}
    int local3; 		// local0{0}

    local0 = argc;
    if (argc == 5) {
        do {
            local2 = local0;
            local1 = local2 - 1;
            local3 = local1;
            if (local2 <= 1) {
                if (local1 == 12) {
                    goto bb0x1000051c;
                }
                local0 = local3;
            }
            else {
bb0x10000498:
                local0 = local1 - 1;
                local3 = local0;
                if (local1 > 2) {
                    goto bb0x1000051c;
                }
                goto bb0x10000498;
            }
            goto bb0x1000051c;
        } while (local0 > 0);
    }
    else {
        if (argc > 5) {
            if (argc == 9) {
bb0x100004ec:
                if (argc != 10) {
bb0x1000050c:
                    goto bb0x1000050c;
                }
                else {
bb0x10000500:
                    goto bb0x10000500;
                }
                goto bb0x100004ec;
            }
            else {
            }
        }
        else {
            if (argc == 2) {
                do {
bb0x10000454:
                } while (argc > 0);
                goto bb0x10000454;
            }
            else {
            }
        }
    }
bb0x1000051c:
    return 13;
}

