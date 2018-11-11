int main(int argc, char *argv[]);

/** address: 0x00010684 */
int main(int argc, char *argv[])
{
    union { __size32; float; } f2; 		// r34

    f2 = *0x1078c;
    printf("Pi is about %.5f\n", f2);
    return 0;
}

