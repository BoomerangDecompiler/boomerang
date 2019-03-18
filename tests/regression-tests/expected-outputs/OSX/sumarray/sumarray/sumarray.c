int main(int argc, char *argv[]);

__size32 a[];

/** address: 0x00001d0c */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 32]
    int local1; 		// m[g1 - 28]

    local0 = 0;
    local1 = 0;
    while (local1 <= 9) {
        local0 += a[local1];
        local1++;
    }
    printf("Sum is %d\n", local0);
    return 0;
}

