int main(int argc, char *argv[]);


/** address: 0x08048390 */
int main(int argc, char *argv[])
{
    int local0; 		// m[esp - 8]

    local0 = 0;
    while (local0 <= 9) {
        local0++;
    }
    printf("%i\n", local0);
    return 0;
}

