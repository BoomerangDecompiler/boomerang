int main(int argc, char *argv[]);

/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 19]

    local0 = 1;
    if (argc <= 1) {
        local0 = 0;
    }
    printf(0x10000834);
    return ROTL(((char) (local0))) & 0xff;
}

