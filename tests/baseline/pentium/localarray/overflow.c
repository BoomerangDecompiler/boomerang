// address: 8048334
int main(int argc, int argv, int envp) {
    int local0; 		// m[esp - 272]

    local0 = 0;
    while (local0 <= 63) {
        *(__size32*)(esp + local0 * 4 - 268) = 0;
        local0++;
    }
    return 0;
}

