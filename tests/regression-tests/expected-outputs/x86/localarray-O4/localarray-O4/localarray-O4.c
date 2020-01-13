int main(int argc, char *argv[]);


/** address: 0x08048340 */
int main(int argc, char *argv[])
{
    unsigned int eax; 		// r24
    unsigned int eax_1; 		// r24{3}
    unsigned int eax_2; 		// r24{5}
    unsigned int local0; 		// eax_1{3}

    eax = 0;
    local0 = eax;
    do {
        eax_1 = local0;
        *(__size32*)(esp + eax_1 * 4 - 268) = 0;
        eax_2 = eax_1 + 1;
        local0 = eax_2;
    } while (eax_1 + 1 <= 63);
    return 0;
}

