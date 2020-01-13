int main(int argc, char *argv[]);


/** address: 0x08048340 */
int main(int argc, char *argv[])
{
    __size32 eax; 		// r24
    __size32 eax_1; 		// r24{5}
    __size32 eax_2; 		// r24{7}
    __size32 local0; 		// eax_1{5}

    eax = 0;
    local0 = eax;
    do {
        eax_1 = local0;
        *(__size8*)(eax_1 + esp - 76) = 0;
        eax_2 = eax_1 + 1;
        local0 = eax_2;
    } while (eax_1 + 1 <= 63);
    return 0;
}

