union { char *[] *; __size32; } a[10];
int main(int argc, char *argv[]);

/** address: 0x0001069c */
int main(int argc, char *argv[])
{
    union { char *[] *; int; } local0; 		// m[o6 - 20]
    int local1; 		// m[o6 - 24]

    local0 = 0;
    local1 = 0;
    while (local1 <= 9) {
        local0 += a[local1];
        local1++;
    }
    printf("Sum is %d\n", local0);
    return 0;
}

