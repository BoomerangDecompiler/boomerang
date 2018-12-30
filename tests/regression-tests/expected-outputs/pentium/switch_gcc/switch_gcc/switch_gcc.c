int main(int argc, char *argv[]);

/** address: 0x08048918 */
int main(int argc, char *argv[])
{
    union { __size32; char *; } local0; 		// m[esp - 8]

    if ((unsigned int)(argc - 2) > 5) {
        local0 = 0x8049482;
    }
    else {
        switch(argc) {
        case 2:
            local0 = 0x8049458;
            break;
        case 3:
            local0 = 0x804945e;
            break;
        case 4:
            local0 = 0x8049466;
            break;
        case 5:
            local0 = 0x804946d;
            break;
        case 6:
            local0 = 0x8049474;
            break;
        case 7:
            local0 = 0x804947a;
            break;
        }
    }
    printf(local0);
    return 0;
}

