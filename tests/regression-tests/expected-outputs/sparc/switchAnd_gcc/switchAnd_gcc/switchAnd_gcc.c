int main(int argc, char *argv[]);

/** address: 0x0001066c */
int main(int argc, char *argv[])
{
    union { char *; __size32; } local0; 		// o0{0}
    int o0; 		// r8
    __size32 o0_1; 		// r8{0}

    o0_1 = 0x107c8;
    local0 = o0_1;
    if (argc <= 1) {
bb0x106b0:
        o0 = local0;
        puts(o0);
    }
    else {
        if ((unsigned int)(argc - 2 & 0x7) > 7) {
        }
        else {
            switch(argc - 2 & 0x7) {
            case 0:
                o0 = 0x107d0;
                local0 = o0;
                goto bb0x106b0;
            case 1:
                o0 = 0x107d8;
                local0 = o0;
                goto bb0x106b0;
            case 2:
                o0 = 0x107e0;
                local0 = o0;
                goto bb0x106b0;
            case 3:
                o0 = 0x107e8;
                local0 = o0;
                goto bb0x106b0;
            case 4:
                o0 = 0x107f0;
                local0 = o0;
                goto bb0x106b0;
            case 5:
bb0x106ec:
                o0 = 0x107f8;
                local0 = o0;
                goto bb0x106b0;
            case 6:
                goto bb0x106ec;
            case 7:
                goto bb0x106ec;
            }
            goto bb0x106b0;
        }
    }
    return 0;
}

