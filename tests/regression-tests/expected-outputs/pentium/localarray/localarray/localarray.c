int main(int argc, char *argv[]);


/** address: 0x08048334 */
int main(int argc, char *argv[])
{
    int local0; 		// m[esp - 272]

    local0 = 0;
    while (local0 <= 63) {
        *(__size32*)(esp + local0 * 4 - 268) = 0;
        local0++;
    }
    return 0;
}

