int main(int argc, char *argv[]);
__size32 proc1(__size32 param1, __size32 param2);


/** address: 0x00001d3c */
int main(int argc, char *argv[])
{
    int g3; 		// r3

    proc1(3, 4);
    printf(/* machine specific */ (int) LR + 680);
    g3 = proc1(5, 6);
    printf(/* machine specific */ (int) LR + 680);
    return g3;
}

/** address: 0x00001d0c */
__size32 proc1(__size32 param1, __size32 param2)
{
    return param1 + param2;
}

