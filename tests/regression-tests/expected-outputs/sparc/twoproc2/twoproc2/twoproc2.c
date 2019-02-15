int main(int argc, char *argv[]);
__size32 proc1(__size32 param1, __size32 param2);


/** address: 0x000106c4 */
int main(int argc, char *argv[])
{
    int o0; 		// r8

    o0 = proc1(3, 4);
    printf("%i\n", o0);
    o0 = proc1(5, 6);
    o0 = printf("%i\n", o0);
    return o0;
}

/** address: 0x000106a0 */
__size32 proc1(__size32 param1, __size32 param2)
{
    return param1 + param2;
}

