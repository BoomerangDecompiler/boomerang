int main(int argc, char *argv[]);
void proc1(int param1, char *param2);


/** address: 0x00001d18 */
int main(int argc, char *argv[])
{
    int g4; 		// r4

    g4 = *(argv + 4);
    proc1(argc, g4);
    printf(/* machine specific */ (int) LR + 716);
    return 0;
}

/** address: 0x00001c74 */
void proc1(int param1, char *param2)
{
    if (param1 <= 2) {
        strlen(param2);
    }
    else {
        strlen(param2);
        strlen(param2);
        printf(/* machine specific */ (int) LR + 868);
    }
    printf(/* machine specific */ (int) LR + 872);
    return;
}

