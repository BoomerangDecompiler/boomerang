int a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
int main(int argc, char *argv[]);

/** address: 0x0001069c */
int main(int argc, char *argv[])
{
    int local0; 		// o2_1{0}
    int o2; 		// r10
    int o2_1; 		// r10{0}
    int o2_4; 		// r10{0}
    int o3; 		// r11

    o3 = 0;
    o2 = 0;
    local0 = o2;
    do {
        o2_1 = local0;
        o2_4 = o2_1 + 1;
        o3 += a[o2_1];
        local0 = o2_4;
    } while (o2_1 + 1 <= 9);
    printf(0x10780);
    return 0;
}

