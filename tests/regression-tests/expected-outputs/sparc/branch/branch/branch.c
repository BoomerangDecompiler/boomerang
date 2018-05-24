int main(int argc, char *argv[]);

/** address: 0x00010a80 */
int main(int argc, char *argv[])
{
    int local0; 		// m[o6 - 20]
    int local1; 		// m[o6 - 24]

    scanf(0x11728);
    scanf(0x11728);
    if (5 == local0) {
        printf(0x11730);
    }
    if (5 != local0) {
        printf(0x11738);
    }
    if (5 > local0) {
        printf(0x11748);
    }
    if (5 <= local0) {
        printf(0x11758);
    }
    if (5 >= local0) {
        printf(0x11768);
    }
    printf(0x11780);
    if ((unsigned int)5 > (unsigned int)local1) {
        printf(0x11788);
    }
    if ((unsigned int)5 <= (unsigned int)local1) {
        printf(0x117a0);
    }
    if ((unsigned int)5 >= (unsigned int)local1) {
        printf(0x117b8);
    }
    if ((unsigned int)5 < (unsigned int)local1) {
        printf(0x117c8);
    }
    if (5 - local0 >= 0) {
        printf(0x117d8);
    }
    if (5 - local0 < 0) {
        printf(0x117e0);
    }
    return 0;
}

