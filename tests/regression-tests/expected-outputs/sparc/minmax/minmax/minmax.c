int main(int argc, char *argv[]);


/** address: 0x00010604 */
int main(int argc, char *argv[])
{
    int g2; 		// r2

    g2 = -2 - (-2 - argc & -1 - (argc >> 31) + ((unsigned int)-2 < (unsigned int)argc));
    printf("MinMax adjusted number of arguments is %d\n", (g2 - 3 & (g2 >> 31) - ((unsigned int)g2 < 3)) + 3);
    return 0;
}

