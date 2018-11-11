#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    struct stat st;
    int res = stat("test/source/stattest.c", &st);
    printf("Stat returns %d; size of file is %d\n",
        res, st.st_size);
    return res;
}

