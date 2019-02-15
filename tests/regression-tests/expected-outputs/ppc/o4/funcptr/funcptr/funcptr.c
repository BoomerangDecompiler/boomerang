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
    printf("Hello, ");
    return;
}

/** address: 0x10000468 */
void world()
{
    puts("world!");
    return;
}

