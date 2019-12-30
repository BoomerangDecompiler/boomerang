int main(int argc, char *argv[]);
__size32 __stat();


/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    int g3; 		// r3
    int g31; 		// r31
    int g4; 		// r4

    g3 = __stat();
    *(__size32*)(g31 + 112) = g3;
    g4 = *(g31 + 112);
    g4 = printf("res: %i\n", g4);
    printf("dev: %i\n", g4);
    g4 = *(g31 + 28);
    printf("ino: %i\n", g4);
    g4 = *(g31 + 32);
    printf("mode: %i\n", g4);
    g4 = *(g31 + 36);
    printf("nlink: %i\n", g4);
    g4 = *(g31 + 40);
    printf("uid: %i\n", g4);
    g4 = *(g31 + 44);
    g4 = printf("gid: %i\n", g4);
    printf("rdev: %i\n", g4);
    g4 = *(g31 + 60);
    printf("size: %i\n", g4);
    g4 = *(g31 + 64);
    printf("blksize: %i\n", g4);
    g4 = *(g31 + 68);
    printf("blocks: %i\n", g4);
    g4 = *(g31 + 72);
    printf("atime: %i\n", g4);
    g4 = *(g31 + 80);
    printf("mtime: %i\n", g4);
    g4 = *(g31 + 88);
    printf("ctime: %i\n", g4);
    return 0;
}

/** address: 0x100006dc */
__size32 __stat()
{
    int g3; 		// r3

    g3 = __xstat();
    return g3;
}

