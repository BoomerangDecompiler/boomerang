int main(int argc, char *argv[]);

/** address: 0x10000418 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 24]

    local0 = 0;
    do {
        local0++;
        printf(0x10000840);
    } while (local0 <= 9);
    printf(0x10000844);
    return 0;
}

