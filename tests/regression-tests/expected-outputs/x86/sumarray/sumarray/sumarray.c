int main(int argc, char *argv[]);

__size32 a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

/** address: 0x08048328 */
int main(int argc, char *argv[])
{
    int local0; 		// m[esp - 8]
    int local1; 		// m[esp - 12]

    local0 = 0;
    local1 = 0;
    while (local1 <= 9) {
        local0 += a[local1];
        local1++;
    }
    printf("Sum is %d\n", local0);
    return 0;
}

