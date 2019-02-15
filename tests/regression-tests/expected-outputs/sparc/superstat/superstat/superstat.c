int main(int argc, char *argv[]);


/** address: 0x000106a8 */
int main(int argc, char *argv[])
{
    struct stat local0; 		// m[o6 - 152]
    int local1; 		// m[o6 - 136]
    int local10; 		// m[o6 - 96]
    int local11; 		// m[o6 - 88]
    int local12; 		// m[o6 - 80]
    int local2; 		// m[o6 - 132]
    int local3; 		// m[o6 - 128]
    int local4; 		// m[o6 - 124]
    int local5; 		// m[o6 - 120]
    int local6; 		// m[o6 - 116]
    int local7; 		// m[o6 - 104]
    int local8; 		// m[o6 - 72]
    int local9; 		// m[o6 - 68]
    int o0; 		// r8
    int o0_1; 		// r8

    o0 = *(argv + 4);
    o0_1 = stat(o0, &local0);
    printf("res: %i\n", o0_1);
    printf("dev: %i\n", local0);
    printf("ino: %i\n", local1);
    printf("mode: %i\n", local2);
    printf("nlink: %i\n", local3);
    printf("uid: %i\n", local4);
    printf("gid: %i\n", local5);
    printf("rdev: %i\n", local6);
    printf("size: %i\n", local7);
    printf("blksize: %i\n", local8);
    printf("blocks: %i\n", local9);
    printf("atime: %i\n", local10);
    printf("mtime: %i\n", local11);
    printf("ctime: %i\n", local12);
    return 0;
}

