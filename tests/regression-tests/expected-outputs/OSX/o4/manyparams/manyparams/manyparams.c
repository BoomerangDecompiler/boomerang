int main(int argc, char *argv[]);

int global_0x00001fec = 0x9999999a;
union { double; __size32; } global_0x00001ff4;

/** address: 0x00001ca8 */
int main(int argc, char *argv[])
{
    double g5_1; 		// r5{3}
    long long g8_1; 		// r8{4}
    double local0; 		// m[g1 - 120]

    g5_1 = *0x1fe8;
    g8_1 = *0x1ff0;
    printf("Many parameters: %d, %.1f, %d, %.1f, %d, %.1f, %d, %.1f\n", 1, g5_1, global_0x00001fec, 2.8025969e-45, g8_1, global_0x00001ff4, 3, local0);
    return 0;
}

