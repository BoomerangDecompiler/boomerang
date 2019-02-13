int main(int argc, char *argv[]);


/** address: 0x00001ca4 */
int main(int argc, char *argv[])
{
    *(__size32*)(/* machine specific */ (int) LR + 888) = 12;
    printf(/* machine specific */ (int) LR + 832);
    printf(/* machine specific */ (int) LR + 840);
    return 0;
}

