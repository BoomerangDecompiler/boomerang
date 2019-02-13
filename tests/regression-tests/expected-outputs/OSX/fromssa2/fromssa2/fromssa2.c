int main(int argc, char *argv[]);


/** address: 0x00001d18 */
int main(int argc, char *argv[])
{
    int local0; 		// m[g1 - 32]

    local0 = 0;
    do {
        local0++;
        printf(/* machine specific */ (int) LR + 696);
    } while (local0 <= 9);
    printf(/* machine specific */ (int) LR + 700);
    return 0;
}

