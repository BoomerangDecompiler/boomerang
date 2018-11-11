int main(int argc, char *argv[]);
__size32 proc1(__size32 param1, __size32 param2, __size32 param3);

/** address: 0x00001d80 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    __size32 g31; 		// r31

    g3 = proc1(/* machine specific */ (int) LR, 11, 4); /* Warning: also results in g31 */
    printf(g31 + 324);
    return g3;
}

/** address: 0x00001d50 */
__size32 proc1(__size32 param1, __size32 param2, __size32 param3)
{
    return param1; /* WARNING: Also returning: g3 := param2 - param3, g31 := param1 */
}

