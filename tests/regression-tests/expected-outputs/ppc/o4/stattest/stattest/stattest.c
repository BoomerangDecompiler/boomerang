int main(int argc, char *argv[]);


/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int g3_1; 		// r3
    int local0; 		// m[g1 - 68]

    g3_1 = __xstat();
    printf("Stat returns %d; size of file is %d\n", g3_1, local0);
    return g3;
}

