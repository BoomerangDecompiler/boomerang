/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

#include <stdio.h>
#include <stdlib.h>


/*
 *         a (main)
 *         |
 *         b<----
 *         |     \
 *   ----->c___   |
 *  / / / \  \ \ /
 * | d  f  h  j l
 * | |  |\ |  |
 * +-e  g/ i  k
 *    \______/
 */

void b(int x);
void c(int x);
void d(int x);
void e(int x);
void f(int x);
void g(int x);
void h(int x);
void i(int x);
void j(int x);
void k(int x);
void l(int x);


int main(int argc, char *argv[])
{
    printf("a(%d)\n", argc);
    b(argc*3);
    return 0;
}


void b(int x)
{
    printf("b(%d)\n", x);
    c(x-1);
}


void c(int x)
{
    printf("c(%d)\n", x);
    switch (x) {
    case 2: d(2); break;
    case 3: f(3); break;
    case 4: h(4); break;
    case 5: j(5); break;
    case 6: l(6); break;
    }
}


void d(int x)
{
    printf("d(%d)\n", x);
    if (x > 1) {
        e(x-1);
    }
}


void e(int x)
{
    printf("e(%d)\n", x);
    c(x >> 1);
}


void f(int x)
{
    printf("f(%d)\n", x);
    if (x > 1) {
        g(x-1);
    }
}


void g(int x)
{
    printf("g(%d)\n", x);
    if (x > 1) {
        f(x-1);
    }
}


void h(int x)
{
    printf("h(%d)\n", x);
    if (x > 0) {
        i(x-1);
    }
}


void i(int x)
{
    printf("i(%d)\n", x);
}


void j(int x)
{
    printf("j(%d)\n", x);
    if (x >= 2) {
        k(x);
    }
}


void k(int x)
{
    printf("k(%d)\n", x);
    if (x >= 2) {
        e(x-1);
    }
}


void l(int x)
{
    printf("l(%d)\n", x);
    if (x >= 2) {
        b(x+2);
    }
}
