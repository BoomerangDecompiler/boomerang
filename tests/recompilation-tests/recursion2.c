/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/*
 * NOTE: this is not the whole source code for the test.
 * There are many additions made at the assembly language
 * level, see test/pentium/recursion2.s
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


int res = 0;       /* Global affected as a side effect from all calls */
int b_c = 3;       /* Number of times to traverse b to c edge */
int c_d = 3;
int d_e = 3;
int e_c = 3;
int c_f = 3;
int f_g = 3;
int g_f = 3;
int c_h = 3;
int h_i = 3;
int c_j = 3;
int j_k = 3;
int k_e = 3;
int c_l = 3;
int l_b = 3;

void b();
void c();
void d();
void e();
void f();
void g();
void h();
void i();
void j();
void k();
void l();


int main(int argc, char *argv[])
{
    b();
    printf("ecx is %d, edx is %d\n", 0, 0);
    printf("res is %d\n", res);
}


void b()
{
    if (--b_c >= 0) c();
    res += 2;
}


void c()
{
    if (--c_d >= 0) d();
    if (--c_f >= 0) f();
    if (--c_h >= 0) h();
    if (--c_j >= 0) j();
    if (--c_l >= 0) l();
    res += 3;
}


void d()
{
    if (--d_e >= 0) e();
    res += 5;
}


void e()
{
    if (--e_c >= 0) c();
    res += 7;
}


void f()
{
    if (--f_g >= 0) g();
    res += 11;
}


void g()
{
    if (--g_f >= 0) f();
    res += 13;
}


void h()
{
    if (--h_i >= 0) i();
    res += 17;
}


void i()
{
    res += 19;
}


void j()
{
    if (--j_k >= 0) k();
    res += 23;
}


void k()
{
    if (--k_e >= 0) e();
    res += 27;
}


void l()
{
    if (--l_b >= 0) b();
    res += 29;
}
