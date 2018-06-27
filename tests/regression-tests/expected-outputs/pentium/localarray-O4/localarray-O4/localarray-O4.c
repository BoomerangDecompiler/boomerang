int main(int argc, char *argv[]);

/** address: 0x08048340 */
int main(int argc, char *argv[])
{
    unsigned int eax; 		// r24
    unsigned int eax_1; 		// r24{0}
    unsigned int eax_4; 		// r24{0}
    unsigned int local0; 		// eax_1{0}

    eax = 0;
    local0 = eax;
    do {
        eax_1 = local0;
        *(__size32*)(esp + eax_1 * 4 - 268) = 0;
        eax_4 = eax_1 + 1;
        local0 = eax_4;
    } while (eax_1 + 1 <= 63);
    return 0;
}

