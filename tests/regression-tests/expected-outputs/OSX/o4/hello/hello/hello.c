int main(int argc, char *argv[]);


/** address: 0x00001d68 */
int main(int argc, char *argv[])
{
    puts(/* machine specific */ (int) LR + 636);
    return 0;
}

