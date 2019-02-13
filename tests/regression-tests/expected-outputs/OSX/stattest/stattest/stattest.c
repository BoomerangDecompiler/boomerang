int main(int argc, char *argv[]);


/** address: 0x00001ce0 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    struct stat local0; 		// m[g1 - 128]

    g3 = stat(/* machine specific */ (int) LR + 712, &local0);
    printf(/* machine specific */ (int) LR + 736);
    return g3;
}

