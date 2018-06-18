int main(int argc, char *argv[]);

/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 36]
    unsigned int local1; 		// m[g1 - 28]

    scanf(0x100009a8);
    scanf(0x100009a8);
    if (5 == local0) {
        printf(0x100009ac);
    }
    if (5 != local0) {
        printf(0x100009b4);
    }
    if (5 > local0) {
        printf(0x100009c0);
    }
    if (5 <= local0) {
        printf(0x100009cc);
    }
    if (5 >= local0) {
        printf(0x100009dc);
    }
    if (5 < local0) {
        printf(0x100009f0);
    }
    if ((unsigned int)5 > local1) {
        printf(0x100009f8);
    }
    if ((unsigned int)5 <= local1) {
        printf(0x10000a0c);
    }
    if ((unsigned int)5 >= local1) {
        printf(0x10000a24);
    }
    if ((unsigned int)5 < local1) {
        printf(0x10000a34);
    }
    if (5 >= local0) {
        printf(0x10000a40);
    }
    if (5 < local0) {
        printf(0x10000a48);
    }
    return 5 - local0;
}

