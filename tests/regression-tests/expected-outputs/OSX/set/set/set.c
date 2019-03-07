int main(int argc, char *argv[]);


/** address: 0x00001d28 */
int main(int argc, char *argv[])
{
    unsigned char local0; 		// m[g1 - 31]
    unsigned char local1; 		// m[g1 - 32]

    local0 = 1;
    if (argc <= 1) {
        local0 = 0;
    }
    local1 = (char) (local0);
    printf("Result is %d\n", (int) (local1));
    return (int) (local1);
}

