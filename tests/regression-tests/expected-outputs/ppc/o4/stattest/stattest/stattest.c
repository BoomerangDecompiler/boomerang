int main(int argc, char *argv[]);

/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    int g3; 		// r3

    g3 = __xstat();
    printf(0x10000850);
    return g3;
}

