int main(int argc, char *argv[]);
void mid();
void fst();

/** address: 0x00010744 */
int main(int argc, char *argv[])
{
    int local0; 		// m[o6 - 24]
    int local1; 		// m[o6 - 28]
    int local2; 		// m[o6 - 20]
    int o0; 		// r8

    local0 = 0;
    mid();
    fst();
    local1 = 0x20a50;
    local2 = 0;
    while (local2 <= 4) {
        o0 = *(unsigned char*)local1;
        local0 += (int)(o0 * 0x1000000) >> 24;
        local1++;
        local2++;
    }
    printf(0x108d0);
    return 0;
}

/** address: 0x000106cc */
void mid()
{
    printf(0x108a0);
    return;
}

/** address: 0x00010708 */
void fst()
{
    printf(0x108b8);
    return;
}

