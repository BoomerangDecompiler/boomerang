#include <stdio.h>

/* x should be a parameter here, since it is used before definition, although only on some paths */
/* y should be a parameter here, since it is returned */

int cparam(int x, int y)
{
    if (x < 0) {
        y = 0;
    }

    return x+y;
}

int main(int argc, char *argv[])
{
    printf("Result is %d\n", cparam(argc-3, 2));
    return 0;
}
