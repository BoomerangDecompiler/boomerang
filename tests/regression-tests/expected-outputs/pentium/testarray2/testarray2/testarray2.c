int main(int argc, char *argv[]);
void mid(__size32 param1);
void fst(__size32 param1);


/** address: 0x080483ac */
int main(int argc, char *argv[])
{
    int edx; 		// r26
    int local0; 		// m[esp - 12]
    char *local2; 		// m[esp - 16]
    int local3; 		// m[esp - 8]

    local0 = 0;
    mid(0x8049654);
    fst(0x804964a);
    local2 = "\2\4\6\b\n";
    local3 = 0;
    while (local3 <= 4) {
        edx = (int) *local2;
        local0 += edx;
        local2++;
        local3++;
    }
    printf("Sum is %d\n", local0);
    return 0;
}

/** address: 0x08048368 */
void mid(__size32 param1)
{
    int eax; 		// r24

    eax = (int) *(param1 + 2);
    printf("Middle elment is %d\n", eax);
    return;
}

/** address: 0x0804838a */
void fst(__size32 param1)
{
    int eax; 		// r24

    eax = (int) *(param1 + 10);
    printf("First element is %d\n", eax);
    return;
}

