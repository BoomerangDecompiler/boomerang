int main(int argc, char *argv[]);

__size32 a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

/** address: 0x08048328 */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    int eax_1; 		// r24{3}
    int eax_2; 		// r24{6}
    int edx; 		// r26
    int edx_1; 		// r26{4}
    int local2; 		// eax_1{3}

    edx = 0;
    eax = 0;
    local2 = eax;
    do {
        edx_1 = edx;
        eax_1 = local2;
        edx = edx_1 + a[eax_1];
        eax_2 = eax_1 + 1;
        local2 = eax_2;
    } while (eax_1 + 1 <= 9);
    printf("Sum is %d\n", edx_1 + a[eax_1]);
    return 0;
}

