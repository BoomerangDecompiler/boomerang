int main(int argc, char *argv[]);


/** address: 0x08048328 */
int main(int argc, char *argv[])
{
    __size32 ebx; 		// r27
    __size32 ebx_1; 		// r27{2}
    __size32 ebx_2; 		// r27{3}
    __size32 local3; 		// ebx_1{2}

    ebx = 0;
    local3 = ebx;
    do {
        ebx_1 = local3;
        ebx_2 = ebx_1 + 1;
        local3 = ebx_2;
        printf("%d ", ebx_1 + 1);
    } while (ebx_1 + 1 <= 9);
    printf("a is %d, x is %d\n", 10, 10);
    return 0;
}

