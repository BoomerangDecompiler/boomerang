#include <sys/stat.h>

int main(int argc, char **argv) {
    struct stat st;
    int res = stat(argv[1], &st);
    printf("res: %i\n", res);
    printf("dev: %i\n", st.st_dev);
    printf("ino: %i\n", st.st_ino);
    printf("mode: %i\n", st.st_mode);
    printf("nlink: %i\n", st.st_nlink);
    printf("uid: %i\n", st.st_uid);
    printf("gid: %i\n", st.st_gid);
    printf("rdev: %i\n", st.st_rdev);
    printf("size: %i\n", st.st_size);
    printf("blksize: %i\n", st.st_blksize);
    printf("blocks: %i\n", st.st_blocks);
    printf("atime: %i\n", st.st_atime);
    printf("mtime: %i\n", st.st_mtime);
    printf("ctime: %i\n", st.st_ctime);
    return 0;
}
