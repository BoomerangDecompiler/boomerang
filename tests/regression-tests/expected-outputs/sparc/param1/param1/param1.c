int main(int argc, char *argv[]);
void cparam(int param1);

/** address: 0x000106a0 */
int main(int argc, char *argv[])
{
    cparam(argc - 3);
    printf(0x10760);
    return 0;
}

/** address: 0x00010688 */
void cparam(int param1)
{
    if (param1 < 0) {
    }
    return;
}

