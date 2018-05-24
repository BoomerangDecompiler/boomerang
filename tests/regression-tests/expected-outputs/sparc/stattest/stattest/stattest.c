int main(int argc, char *argv[]);

/** address: 0x000106a8 */
int main(int argc, char *argv[])
{
    struct stat local0; 		// m[o6 - 152]
    int o0; 		// r8

    o0 = stat(0x10780, &local0);
    printf(0x10798);
    return o0;
}

