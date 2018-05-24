int main(int argc, char *argv[]);
__size32 proc1(__size32 param1, __size32 param2, __size32 param3);

/** address: 0x00001d3c */
int main(int argc, char *argv[])
{
    __size64 f29; 		// r61
    int g3; 		// r3

    f29 = proc1(f29, 3, 4);
    printf(/* machine specific */ (int) LR + 680);
    g3 = proc1(f29, 5, 6);
    printf(/* machine specific */ (int) LR + 680);
    return g3;
}

/** address: 0x00001d0c */
__size32 proc1(__size32 param1, __size32 param2, __size32 param3)
{
    return param1; /* WARNING: Also returning: g3 := param2 + param3, f29_1 := param1 */
}

