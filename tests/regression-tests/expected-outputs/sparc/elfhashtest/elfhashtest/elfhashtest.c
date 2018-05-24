int main(int argc, char *argv[]);
void elf_hash(union { __size32; char *; } param1);

/** address: 0x00010678 */
int main(int argc, char *argv[])
{
    elf_hash(0x107a8);
    printf(0x107b0);
    return 0;
}

/** address: 0x000106b4 */
void elf_hash(union { __size32; char *; } param1)
{
    int o3; 		// r11

    o3 = (int) *param1;
    if (o3 != 0) {
        o3 = (int) *(param1 + 1);
    }
    return;
}

