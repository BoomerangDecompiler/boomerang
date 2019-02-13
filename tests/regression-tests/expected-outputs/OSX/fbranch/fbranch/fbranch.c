int main(int argc, char *argv[]);


/** address: 0x00001bb8 */
int main(int argc, char *argv[])
{
    void *g1; 		// r1
    float local0; 		// m[g1 - 44]

    scanf(/* machine specific */ (int) LR + 972);
    printf(/* machine specific */ (int) LR + 976);
    if (0x40a00000 == local0) {
        printf(/* machine specific */ (int) LR + 996);
    }
    if (0x40a00000 != local0) {
        printf(/* machine specific */ (int) LR + 1004);
    }
    if (5. > local0) {
        printf(/* machine specific */ (int) LR + 1016);
    }
    if (0x40a00000 == local0) {
        printf(/* machine specific */ (int) LR + 1028);
    }
    if (0x40a00000 == local0) {
        printf(/* machine specific */ (int) LR + 1044);
    }
    if (5. < local0) {
        printf(/* machine specific */ (int) LR + 1064);
    }
    return g1 - 44;
}

