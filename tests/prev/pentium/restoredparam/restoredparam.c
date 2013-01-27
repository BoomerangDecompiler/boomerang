void twice();

// address: 0x804837c
int main(int argc, char *argv[], char *envp[]) {
    twice();
    proc1();
    return 0;
}

// address: 0x8048396
void twice() {
    return;
}

