int main(int argc, char *argv[]);

union { double; __size32; } global_0x00001fe8;
int global_0x00001fec = 0x9999999a;
long long global_0x00001ff0 = 0x400199999999999aLL;
union { double; __size32; } global_0x00001ff4;

/** address: 0x00001c78 */
int main(int argc, char *argv[])
{
    double local0; 		// m[g1 - 104]

    printf("Many parameters: %d, %.1f, %d, %.1f, %d, %.1f, %d, %.1f\n", 1, global_0x00001fe8, global_0x00001fec, 2.8025969e-45, global_0x00001ff0, global_0x00001ff4, 3, local0);
    return 0;
}

