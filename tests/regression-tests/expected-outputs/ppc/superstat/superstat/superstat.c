int main(int argc, char *argv[]);
__size32 __stat();


/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    int g3_1; 		// r3
    int g4; 		// r4
    int local0; 		// m[g1 - 116]
    int local1; 		// m[g1 - 112]
    int local10; 		// m[g1 - 56]
    int local2; 		// m[g1 - 108]
    int local3; 		// m[g1 - 104]
    int local4; 		// m[g1 - 100]
    int local5; 		// m[g1 - 84]
    int local6; 		// m[g1 - 80]
    int local7; 		// m[g1 - 76]
    int local8; 		// m[g1 - 72]
    int local9; 		// m[g1 - 64]

    g3_1 = __stat();
    g4 = printf("res: %i\n", g3_1);
    printf("dev: %i\n", g4);
    printf("ino: %i\n", local0);
    printf("mode: %i\n", local1);
    printf("nlink: %i\n", local2);
    printf("uid: %i\n", local3);
    g4 = printf("gid: %i\n", local4);
    printf("rdev: %i\n", g4);
    printf("size: %i\n", local5);
    printf("blksize: %i\n", local6);
    printf("blocks: %i\n", local7);
    printf("atime: %i\n", local8);
    printf("mtime: %i\n", local9);
    printf("ctime: %i\n", local10);
    return 0;
}

/** address: 0x100006dc */
__size32 __stat()
{
    int g3; 		// r3

    g3 = __xstat();
    return g3;
}

