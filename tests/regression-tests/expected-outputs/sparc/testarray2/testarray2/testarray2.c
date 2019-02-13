int main(int argc, char *argv[]);
void mid(__size32 param1);
void fst(__size32 param1);


/** address: 0x00010744 */
int main(int argc, char *argv[])
{
    int local0; 		// m[o6 - 24]
    unsigned char *local1; 		// m[o6 - 28]
    int local2; 		// m[o6 - 20]
    int o0; 		// r8

    local0 = 0;
    mid(0x20a50);
    fst(0x20a46);
    local1 = 0x20a50;
    local2 = 0;
    while (local2 <= 4) {
        o0 = *(unsigned char*)local1;
        local0 += o0 << 24 >> 24;
        local1++;
        local2++;
    }
    printf("Sum is %d\n", local0);
    return 0;
}

/** address: 0x000106cc */
void mid(__size32 param1)
{
    int o0; 		// r8

    o0 = *(unsigned char*)(param1 + 2);
    printf("Middle elment is %d\n", o0 << 24 >> 24);
    return;
}

/** address: 0x00010708 */
void fst(__size32 param1)
{
    int o0; 		// r8

    o0 = *(unsigned char*)(param1 + 10);
    printf("First element is %d\n", o0 << 24 >> 24);
    return;
}

