int main(int argc, char *argv[]);
void proc1();

/** address: 0x000106c4 */
int main(int argc, char *argv[])
{
    int o0; 		// r8

    proc1();
    printf(0x107c0);
    proc1();
    o0 = printf(0x107c0);
    return o0;
}

/** address: 0x000106a0 */
void proc1()
{
    return;
}

