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
bb0x804894c:
            local0 = 0x8049458;
            goto bb0x804894c;
        case 3:
bb0x8048954:
            local0 = 0x804945e;
            goto bb0x8048954;
        case 4:
bb0x804895c:
            local0 = 0x8049466;
            goto bb0x804895c;
        case 5:
bb0x8048964:
            local0 = 0x804946d;
            goto bb0x8048964;
        case 6:
bb0x804896c:
            local0 = 0x8049474;
            goto bb0x804896c;
        case 7:
bb0x8048974:
            local0 = 0x804947a;
            goto bb0x8048974;
        }
    }
    printf(local0);
    return 0;
}

