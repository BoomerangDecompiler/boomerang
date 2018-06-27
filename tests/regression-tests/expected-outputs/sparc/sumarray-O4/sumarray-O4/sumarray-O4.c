__size32 a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
int main(int argc, char *argv[]);

/** address: 0x0001069c */
int main(int argc, char *argv[])
{
    unsigned int local0; 		// o2_1{0}
    int o2; 		// r10
    unsigned int o2_1; 		// r10{0}
    unsigned int o2_4; 		// r10{0}
    int o3; 		// r11
    __size32 o3_1; 		// r11{0}

    o3 = 0;
    o2 = 0;
    local0 = o2;
    do {
        o2_1 = local0;
        o3_1 = o3;
        o2_4 = o2_1 + 1;
        o3 = o3_1 + a[o2_1];
        local0 = o2_4;
    } while (o2_1 + 1 <= 9);
    printf("Sum is %d\n", o3_1 + a[o2_1]);
    return 0;
}

