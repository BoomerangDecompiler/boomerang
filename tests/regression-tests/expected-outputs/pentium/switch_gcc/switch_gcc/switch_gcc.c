int main(int argc, char *argv[]);

/** address: 0x08048918 */
int main(int argc, char *argv[])
{
    __size32 local0; 		// m[esp - 8]

    if ((unsigned int)(argc - 2) > 5) {
        local0 = 0x8049482;
    }
    else {
        switch(argc) {
        case 2:
            local0 = 0x8049458;
            goto bb0x8048981;
        case 3:
            local0 = 0x804945e;
            goto bb0x8048981;
        case 4:
            local0 = 0x8049466;
            goto bb0x8048981;
        case 5:
            local0 = 0x804946d;
            goto bb0x8048981;
        case 6:
            local0 = 0x8049474;
            goto bb0x8048981;
        case 7:
            local0 = 0x804947a;
            goto bb0x8048981;
        }
    }
bb0x8048981:
    printf(local0);
    return 0;
}

