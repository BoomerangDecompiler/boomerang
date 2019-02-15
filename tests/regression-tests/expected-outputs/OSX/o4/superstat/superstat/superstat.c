int main(int argc, char *argv[]);


/** address: 0x00001bc0 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    struct stat local0; 		// m[g1 - 112]

    g3 = *(argv + 4);
    stat(g3, &local0);
    printf(/* machine specific */ (int) LR + 900);
    printf(/* machine specific */ (int) LR + 912);
    printf(/* machine specific */ (int) LR + 924);
    printf(/* machine specific */ (int) LR + 936);
    printf(/* machine specific */ (int) LR + 948);
    printf(/* machine specific */ (int) LR + 960);
    printf(/* machine specific */ (int) LR + 972);
    printf(/* machine specific */ (int) LR + 984);
    printf(/* machine specific */ (int) LR + 996);
    printf(/* machine specific */ (int) LR + 1008);
    printf(/* machine specific */ (int) LR + 1024);
    printf(/* machine specific */ (int) LR + 1036);
    printf(/* machine specific */ (int) LR + 1048);
    printf(/* machine specific */ (int) LR + 1060);
    return 0;
}

