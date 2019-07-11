int main(int argc, char *argv[]);

int global_0x00001fec = 0x9999999a;
union { double; __size32; } global_0x00001ff4;

/** address: 0x00001ca8 */
int main(int argc, char *argv[])
{
    int g5; 		// r5
    int g8; 		// r8
    double local0; 		// m[g1 - 120]

    g5 = *0x1fe8;
    g8 = *0x1ff0;
    printf("Many parameters: %d, %.1f, %d, %.1f, %d, %.1f, %d, %.1f\n", 1, g5, global_0x00001fec, 2.8025969e-45, g8, global_0x00001ff4, 3, local0);
    return 0;
}

