/* phi2.c. Tests transforming out of SSA form. Compile with:
   gcc -o phi2 /path/to/phi2.c    (i.e. no optimisation) */
#include <stdio.h>
#include <string.h>

int proc1 (int x, char* s)
{
    int z;
    if (x > 2) {
        x = strlen(s);
        z = strlen(s);
        printf("%d", x + z);
    }
    else {
        x = strlen(s);
    }
    printf("%d, %d", x, z);
    return x;
}

int main (int argc, char* argv[])
{
    printf("%d\n", proc1(argc, argv[1]));
    return 0;
}
