int main(int argc, char *argv[]);


/** address: 0x0804835c */
int main(int argc, char *argv[])
{
    int eax; 		// r24
    struct stat local0; 		// m[esp - 108]
    int local1; 		// m[esp - 96]
    int local10; 		// m[esp - 52]
    int local11; 		// m[esp - 44]
    int local12; 		// m[esp - 36]
    char *local13; 		// m[esp - 120]
    int local2; 		// m[esp - 92]
    int local3; 		// m[esp - 88]
    int local4; 		// m[esp - 84]
    int local5; 		// m[esp - 80]
    int local6; 		// m[esp - 76]
    int local7; 		// m[esp - 64]
    int local8; 		// m[esp - 60]
    int local9; 		// m[esp - 56]

    local13 = *(argv + 4);
    eax = stat(3, local13, &local0);
    printf("res: %i\n", eax);
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

