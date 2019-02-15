int main(int argc, char *argv[]);


/** address: 0x00010684 */
int main(int argc, char *argv[])
{
    int local0; 		// m[o6 - 24]
    int local1; 		// m[o6 - 20]
    int o0; 		// r8

    local0 = 0;
    local1 = 0;
    while (local1 <= 4) {
        o0 = *(unsigned char*)(local1 + 0x20930);
        local0 += o0 << 24 >> 24;
        local1++;
    }
    printf("Sum is %d\n", local0);
    return 0;
}

