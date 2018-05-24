int main(int argc, char *argv[]);

/** address: 0x000106d0 */
int main(int argc, char *argv[])
{
    __size32 f1; 		// r33
    __size32 f5; 		// r37
    int f6; 		// r38
    int f7; 		// r39
    int f8; 		// r40
    int f9; 		// r41
    union { __size32; float; } local0; 		// m[o6 - 20]

    scanf(0x108c8);
    printf(0x108d0);
    f1 = *0x10938;
    if (f1 != local0) {
bb0x10768:
        puts(0x108e8);
    }
    else {
        puts(0x108f8);
        f5 = *0x10938;
        if (f5 != local0) {
            goto bb0x10768;
        }
    }
    f6 = *0x10938;
    if (f6 > local0) {
        puts(0x10900);
    }
    else {
    }
    f7 = *0x10938;
    if (f7 <= local0) {
        puts(0x10908);
    }
    f8 = *0x10938;
    if (f8 >= local0) {
        puts(0x10918);
    }
    f9 = *0x10938;
    if (f9 < local0) {
        puts(0x10930);
    }
    return 0x10800;
}

