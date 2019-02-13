int main(int argc, char *argv[]);


/** address: 0x00001cf4 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    struct stat local0; 		// m[g1 - 112]

    g3 = stat(/* machine specific */ (int) LR + 704, &local0);
    printf(/* machine specific */ (int) LR + 728);
    return g3;
}

