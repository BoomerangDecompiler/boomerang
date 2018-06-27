int main(int argc, char *argv[]);

/** address: 0x08048340 */
int main(int argc, char *argv[])
{
    __size32 eax; 		// r24
    __size32 eax_1; 		// r24{0}
    __size32 eax_4; 		// r24{0}
    __size32 local0; 		// eax_1{0}

    eax = 0;
    local0 = eax;
    do {
        eax_1 = local0;
        *(__size8*)(eax_1 + esp - 76) = 0;
        eax_4 = eax_1 + 1;
        local0 = eax_4;
    } while (eax_1 + 1 <= 63);
    return 0;
}

