int main(int argc, char *argv[]);
void hello();
void world();

/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    hello();
    world();
    return 0;
}

/** address: 0x10000490 */
void hello()
{
    printf(0x10000860);
    return;
}

/** address: 0x10000468 */
void world()
{
    puts(0x10000858);
    return;
}

