int main(int argc, char *argv[]);


/** address: 0x00001b90 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    struct stat local0; 		// m[g1 - 128]

    g3 = *(argv + 4);
    stat(g3, &local0);
    printf(/* machine specific */ (int) LR + 940);
    printf(/* machine specific */ (int) LR + 952);
    printf(/* machine specific */ (int) LR + 964);
    printf(/* machine specific */ (int) LR + 976);
    printf(/* machine specific */ (int) LR + 988);
    printf(/* machine specific */ (int) LR + 1000);
    printf(/* machine specific */ (int) LR + 1012);
    printf(/* machine specific */ (int) LR + 1024);
    printf(/* machine specific */ (int) LR + 1036);
    printf(/* machine specific */ (int) LR + 1048);
    printf(/* machine specific */ (int) LR + 1064);
    printf(/* machine specific */ (int) LR + 1076);
    printf(/* machine specific */ (int) LR + 1088);
    printf(/* machine specific */ (int) LR + 1100);
    return 0;
}

