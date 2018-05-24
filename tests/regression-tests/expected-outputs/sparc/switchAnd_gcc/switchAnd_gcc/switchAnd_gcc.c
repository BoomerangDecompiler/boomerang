int main(int argc, char *argv[]);

/** address: 0x0001066c */
int main(int argc, char *argv[])
{
    int local0; 		// o0{0}
    int o0; 		// r8
    int o0_1; 		// r8{0}

    o0_1 = 0x107c8;
    local0 = o0_1;
    if (argc <= 1) {
        o0 = local0;
        puts(o0);
    }
    else {
        if ((unsigned int)(argc - 2 & 0x7) > (unsigned int)7) {
        }
        else {
bb0x10694:
            switch(argc - 2 & 0x7) {
            case 0:
bb0x106a8:
                o0 = 0x107d0;
                local0 = o0;
                goto bb0x106a8;
            case 1:
bb0x106bc:
                o0 = 0x107d8;
                local0 = o0;
                goto bb0x106bc;
            case 2:
bb0x106c8:
                o0 = 0x107e0;
                local0 = o0;
                goto bb0x106c8;
            case 3:
bb0x106d4:
                o0 = 0x107e8;
                local0 = o0;
                goto bb0x106d4;
            case 4:
bb0x106e0:
                o0 = 0x107f0;
                local0 = o0;
                goto bb0x106e0;
            case 5:
bb0x106ec:
                o0 = 0x107f8;
                local0 = o0;
                goto bb0x106ec;
            case 6:
                goto bb0x10694;
            case 7:
                goto bb0x10694;
            }
            goto bb0x10694;
        }
    }
    return 0;
}

