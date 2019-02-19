int main(int argc, char *argv[]);
__size32 b(int param1);
__size32 c(unsigned int param1);
__size32 d(int param1);
__size32 f(int param1);
__size32 h(int param1);
__size32 j(int param1);
__size32 l(union { int; __size32 *; } param1);
__size32 e(int param1);
__size32 g(union { int; __size32 *; } param1);
__size32 i(int param1);
__size32 k(int param1);

union { int; __size32 *; } global_0x080486c4[];

/** address: 0x0804837c */
int main(int argc, char *argv[])
{
    printf("a(%d)\n", argc);
    b(argc * 3);
    return 0;
}

/** address: 0x080483c7 */
__size32 b(int param1)
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    printf("b(%d)\n", param1);
    eax = c(param1 - 1); /* Warning: also results in ecx, edx */
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x080483f2 */
__size32 c(unsigned int param1)
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    eax = printf("c(%d)\n", param1); /* Warning: also results in ecx, edx */
    if (param1 <= 6) {
        eax = global_0x080486c4[param1];
        switch(param1) {
        case 0:
        case 1:
        case 2:
            eax = d(2); /* Warning: also results in ecx, edx */
            break;
        case 3:
            eax = f(3); /* Warning: also results in ecx, edx */
            break;
        case 4:
            eax = h(4); /* Warning: also results in ecx, edx */
            break;
        case 5:
            eax = j(5); /* Warning: also results in ecx, edx */
            break;
        case 6:
            eax = l(6); /* Warning: also results in ecx, edx */
            break;
        }
    }
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x0804846a */
__size32 d(int param1)
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    eax = printf("d(%d)\n", param1); /* Warning: also results in ecx, edx */
    if (param1 > 1) {
        eax = e(param1 - 1); /* Warning: also results in ecx, edx */
    }
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x080484c7 */
__size32 f(int param1)
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    eax = printf("f(%d)\n", param1); /* Warning: also results in ecx, edx */
    if (param1 > 1) {
        eax = g(param1 - 1); /* Warning: also results in ecx, edx */
    }
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x08048529 */
__size32 h(int param1)
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26
    int local2; 		// m[esp - 28]

    eax = printf("h(%d)\n", param1); /* Warning: also results in ecx, edx */
    if (param1 > 0) {
        local2 = param1 - 1;
        eax = i(param1 - 1); /* Warning: also results in ecx, edx */
    }
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x08048575 */
__size32 j(int param1)
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    eax = printf("j(%d)\n", param1); /* Warning: also results in ecx, edx */
    if (param1 > 1) {
        eax = k(param1); /* Warning: also results in ecx, edx */
    }
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x080485d5 */
__size32 l(union { int; __size32 *; } param1)
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    eax = printf("l(%d)\n", param1); /* Warning: also results in ecx, edx */
    if (param1 > 1) {
        eax = b(param1 + 2); /* Warning: also results in ecx, edx */
    }
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x0804849b */
__size32 e(int param1)
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    printf("e(%d)\n", param1);
    eax = c(param1 >> 1); /* Warning: also results in ecx, edx */
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x080484f8 */
__size32 g(union { int; __size32 *; } param1)
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    eax = printf("g(%d)\n", param1); /* Warning: also results in ecx, edx */
    if (param1 > 1) {
        eax = f(param1 - 1); /* Warning: also results in ecx, edx */
    }
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x0804855a */
__size32 i(int param1)
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    eax = printf("i(%d)\n", param1); /* Warning: also results in ecx, edx */
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

/** address: 0x080485a4 */
__size32 k(int param1)
{
    int eax; 		// r24
    int ecx; 		// r25
    int edx; 		// r26

    eax = printf("k(%d)\n", param1); /* Warning: also results in ecx, edx */
    if (param1 > 1) {
        eax = e(param1 - 1); /* Warning: also results in ecx, edx */
    }
    return eax; /* WARNING: Also returning: ecx := ecx, edx := edx */
}

