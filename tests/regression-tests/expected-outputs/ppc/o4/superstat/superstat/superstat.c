int main(int argc, char *argv[]);


/** address: 0x10000440 */
int main(int argc, char *argv[])
{
    int g3_1; 		// r3
    int g4; 		// r4
    int local0; 		// m[g1 - 84]
    int local1; 		// m[g1 - 80]
    int local10; 		// m[g1 - 24]
    int local2; 		// m[g1 - 76]
    int local3; 		// m[g1 - 72]
    int local4; 		// m[g1 - 68]
    int local5; 		// m[g1 - 52]
    int local6; 		// m[g1 - 48]
    int local7; 		// m[g1 - 44]
    int local8; 		// m[g1 - 40]
    int local9; 		// m[g1 - 32]

    g3_1 = __xstat();
    printf("res: %i\n", g3_1);
    printf("dev: %i\n", 0x10000000);
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

