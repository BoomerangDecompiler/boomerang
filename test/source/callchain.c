/* Tests cases where the only thing defining a parameter is the return location
    from an earlier call (tricky for Sparc)
    Tests "don't bother to pop the last call's argument" for Pentium */
/* Compile with -fno-inline or equivalent */

#include <stdio.h>

int add5(int arg1)
{
    return arg1 + 5;
}
int add10(int arg2)
{
    return 10 + arg2;
}
int add15(int arg3)
{
    return 5  + arg3 + 10;
}

void printarg(int arg4)
{
    printf("Fifty five is %d\n", arg4);
}

int main(int argc)
{
    printarg( add5( add10( add15( 25 ))));
    return 0;
}
