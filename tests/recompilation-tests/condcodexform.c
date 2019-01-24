/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

/*
 * 28 Aug 00 - Mike: Changed second test from "if (args)" to "if (argc > 1")
 *        since if args is calculated incorrectly, it was still printing "Pass"
 */

#include <stdio.h>

void func1() {}
void func2() {}
void func3() {}
void func4() {}
void func5() {}
void func6() {}
void func7() {}
void func8() {}

typedef void voidfcn();

int main(int argc, char* argv[])
{

    /* This test is inspired by the 126.gcc spec benchmark, near the top of
    the emit_case_nodes() function. In the sparc source version, it's a great
    test of transforming the cfg to ensure that each use of flags has one
    definition. In pentium source form, it's not so great, but it still
    tests a few set<cc> instructions, so it's worth while for that.
    The original source and object follow below.

    Usage: condcodexform [x]

    Test by passing a command line argument (not used), and not passing it.
    In case case, the result should be "Pass".

    Original code from 126.gcc:

    int unsignedp = TREE_UNSIGNED (index_type);
    typedef rtx rtx_function ();
    rtx_function *gen_bgt_pat = unsignedp ? gen_bgtu : gen_bgt;
    rtx_function *gen_bge_pat = unsignedp ? gen_bgeu : gen_bge;
    rtx_function *gen_blt_pat = unsignedp ? gen_bltu : gen_blt;
    rtx_function *gen_ble_pat = unsignedp ? gen_bleu : gen_ble;

    7834c:  80 90 00 1b        orcc         %g0, %i3, %g0
    78350:  02 80 00 04        be           0x78360
    78354:  a8 10 00 1a        mov          %i2, %l4
    78358:  10 80 00 02        ba           0x78360
    7835c:  a8 10 00 19        mov          %i1, %l4
    78360:  22 80 00 04        be,a    0x78370
    78364:  e4 03 a0 90        ld           [%sp + 144], %l2
    78368:  10 80 00 02        ba           0x78370
    7836c:  e4 03 a0 94        ld           [%sp + 148], %l2
    78370:  22 80 00 04        be,a    0x78380
    78374:  e2 03 a0 98        ld           [%sp + 152], %l1
    78378:  10 80 00 02        ba           0x78380
    7837c:  e2 03 a0 9c        ld           [%sp + 156], %l1
    78380:  02 80 00 04        be           0x78390
    78384:  90 10 00 1c        mov          %i4, %o0
    78388:  10 80 00 03        ba           0x78394
    7838c:  a0 10 00 1d        mov          %i5, %l0
    78390:  e0 03 a0 a0        ld           [%sp + 160], %l0
    78394:  92 10 00 15        mov          %l5, %o1
    */

    voidfcn *f1, *f2, *f3, *f4;

    int pass = 0;
    int args;

    args = (argc > 1);          /* True if given args */
    f1 = args ? func1 : func2;
    f2 = args ? func3 : func4;
    f3 = args ? func5 : func6;
    f4 = args ? func7 : func8;
    if (argc > 1)
        pass = ((f1 == func1) && (f2 == func3) && (f3 == func5) &&
            (f4 == func7));
    else
        pass = ((f1 == func2) && (f2 == func4) && (f3 == func6) &&
            (f4 == func8));

    if (pass) printf("Pass\n");
    else printf("Failed!\n");

    return !pass;
}
