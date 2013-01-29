__size32 a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

// address: 1069c
int main(int argc, char *argv[], char *envp[]) {
    int o2; 		// r10
    unsigned int o2_1; 		// r10{76}
    int o3; 		// r11
    __size32 o3_1; 		// r11{77}

    o3 = 0;
    o2 = 0;
    do {
        o2_1 = o2;
        o3_1 = o3;
        o2 = o2_1 + 1;
        o3 = o3_1 + a[o2_1];
    } while (o2_1 + 1 <= 9);
    printf("Sum is %d\n", o3_1 + a[o2_1]);
    return 0;
}

