int main(int argc, char *argv[]);


/** address: 0x00001d28 */
int main(int argc, char *argv[])
{
    unsigned char local0; 		// m[g1 - 31]

    local0 = 1;
    if (argc <= 1) {
        local0 = 0;
    }
    printf(/* machine specific */ (int) LR + 688);
    return (int) ((char) (local0));
}

