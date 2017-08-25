#define sign_extend(N, SIZE)    (((int)((N) << (sizeof(unsigned) * 8 - (SIZE)))) >> (sizeof(unsigned) * 8 - (SIZE)))
#include <assert.h>

// #line 2 "machine/hppa/disassembler.m"

/***************************************************************************/ /**
 * \file     disassembler.m
 * OVERVIEW: Matcher file for a stand alone disassembler tool
 *
 * (C) 2000-2001 The University of Queensland, BT group
 *============================================================================*/

/*
 * $Revision$
 *
 * Simon Long 2000.11.30
 * 03 May 01 - Mike: Added constructor names (use -c); don't truncate opcode
 *              names to 3 chars
 * 08 May 01 - Mike: Many mods to work with Cristina's disassembler model
 * 09 May 01 - Mike: %ld -> %d (prevents pages of warnings) with gcc 2.95
 * 18 Jun 01 - Mike: Added floating point instructions
 * 28 Jun 01 - Mike: Major rewrite of integer loads and stores for 1.1 syntax
 * 20 Jul 01 - Mike: Some necessary changes due to Simon's revisions in .spec
 * 23 Jul 01 - Simon: Continued the changes due to revisions in .spec; use
 *              macros to simplify code; separate building of the opcode name
 *              (including completers) using apre and astr char ptrs
 * 24 Jul 01 - Simon: bug fixes (e.g. LDO, etc)
 * 27 Jul 01 - Mike: Display floating registers greater than 32 as frXXR
 * 06 Aug 01 - Mike: Added add[i]b_all; fixed MTCTL (no ct_06 field now)
 * 07 Aug 01 - Mike: Reinstated dis_c_addr for c_s_addr_m[ab] (fixes "STB,ma")
 * 07 Aug 01 - Simon: dis_addr() gone completely - [addr] => [xd,s,b]
 * 20 Aug 01 - Mike: Fixed the CONS() for floating point loads and stores
 */

#include "boomerang/global.h"
#include "boomerang/include/decoder.h"

#include "boomerang/hppa-names.h"

// globals
extern char _assembly[];
char        aprefix[256];
char        adata[256];
char        *apre;
char        *astr;
char        *cmpltsep = "."; // ??

// Prototypes
const char *GetSym(unsigned pc);
const char *GetReloc(unsigned pc);

#define ANAME    apre += sprintf(apre, "%s", name);
// #define APREF(x)    apre += sprintf( apre, "%s", x );
// #define AARGs(x)    astr += sprintf( astr, "%s", x );
// #define AARGd(x)    astr += sprintf( astr, "%d", x );
// #define AARGf(f, x) astr += sprintf( astr, " ## f ## ", x );
// #define Acom        astr += sprintf( astr, "," );
#define CONS(x)      strcat(constrName, x);
#define IGNORE(x)    not_used(*(int *)&x);

// The below is used to quelch annoying "variable not used" warnings
void not_used(int unwanted)
{
    unwanted = 0;
}


DWord getDword(ADDRESS lc)

/* get4Bytes - returns next 4-Byte from image pointed to by lc.
 * Fetch in a big-endian manner  */
{
    return (DWord)((((((*(Byte *)lc << 8) + *(Byte *)(lc + 1)) << 8) + *(Byte *)(lc + 2)) << 8) + *(Byte *)(lc + 3));
}


static char killBuffer[32];

// Find and kill any dots in the opcode name (and all chars thereafter)
char *killDot(char *str)
{
    strcpy(killBuffer, str);
    char *p = strchr(killBuffer, '.');

    if (p) {
        *p = '\0';
    }

    return killBuffer;
}


static char shexBuffer[32];

char *signedHex(int disp)
{
    if (disp < 0) {
        sprintf(shexBuffer, "-0x%x", -disp);
    }
    else {
        sprintf(shexBuffer, "0x%x", disp);
    }

    return shexBuffer;
}


// This function returns 0 or 8; the 8 is to access the last half of arrays
// of conditions (e.g. CMPIBF, or the "f" bit of ariths)
int dis_c_c_n(ADDRESS hostpc)
{
    int result = 0;

    // #line 105 "machine/hppa/disassembler.m"
    {
        dword MATCH_p =

#line 105 "machine/hppa/disassembler.m"
            hostpc;
        unsigned MATCH_w_32_0;
        {
            MATCH_w_32_0 = getDword(MATCH_p);

            switch ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */)
            {
            case 0:
            case 1:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 31:
            case 38:
            case 46:
            case 48:
            case 49:
            case 54:
            case 55:
            case 56:
            case 57:
            case 58:
            case 60:
            case 61:
            case 62:
            case 63:
                goto MATCH_label_h0;
                break;

            case 2:

                switch ((MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */)
                {
                case 0:
                case 4:
                case 5:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                case 17:
                case 19:
                case 20:
                case 21:
                case 22:
                case 23:
                case 24:
                case 25:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:
                case 31:
                case 34:
                case 38:
                case 39:
                case 40:
                case 41:
                case 42:
                case 43:
                case 46:
                case 47:
                case 48:
                case 51:
                case 52:
                case 56:
                case 57:
                case 58:
                case 59:
                case 60:

                    if ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ == 1) {
                        goto MATCH_label_h2; /*opt-block+*/
                    }
                    else {
                        goto MATCH_label_h1; /*opt-block+*/
                    }

                    break;

                case 1:
                case 2:
                case 3:
                case 6:
                case 18:
                case 32:
                case 33:
                case 35:
                case 36:
                case 37:
                case 44:
                case 45:
                case 49:
                case 50:
                case 53:
                case 54:
                case 55:
                case 61:
                case 62:
                case 63:
                    goto MATCH_label_h0;
                    break;

                default:
                    assert(0);
                } /* (MATCH_w_32_0 >> 6 & 0x3f) -- ext6_20 at 0 --*/

                break;

            case 32:
            case 33:
            case 39:
            case 40:
            case 41:
                goto MATCH_label_h1;
                break;

            case 34:
            case 35:
            case 42:
            case 43:
            case 47:
                goto MATCH_label_h2;
                break;

            case 36:

                if ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ == 1) {
                    goto MATCH_label_h2; /*opt-block+*/
                }
                else {
                    goto MATCH_label_h1; /*opt-block+*/
                }

                break;

            case 37:
            case 44:
            case 45:

                if (((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ == 1) &&
                    ((0 <= (MATCH_w_32_0 >> 11 & 0x1)) /* ext_20 at 0 */ &&
                     ((MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ < 2))) {
                    goto MATCH_label_h2; /*opt-block+*/
                }
                else {
                    goto MATCH_label_h1; /*opt-block+*/
                }

                break;

            case 50:
            case 51:

                if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) {
                    goto MATCH_label_h2; /*opt-block+*/
                }
                else {
                    goto MATCH_label_h1; /*opt-block+*/
                }

                break;

            case 52:

                if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) {
                    if (((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 0) ||
                        (2 <= (MATCH_w_32_0 >> 13 & 0x7)) /* c3_16 at 0 */ &&
                        ((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ < 8) ||
                        ((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 1) &&
                        ((MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 0) &&
                        ((MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 1)) {
                        goto MATCH_label_h0; /*opt-block+*/
                    }
                    else {
                        goto MATCH_label_h2; /*opt-block+*/ /*opt-block+*/
                    }
                }
                else if (((MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 0) &&
                         ((MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 1)) {
                    goto MATCH_label_h0; /*opt-block+*/
                }
                else {
                    goto MATCH_label_h1; /*opt-block+*/ /*opt-block+*/
                }

                break;

            case 53:

                if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) {
                    if (((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 0) ||
                        (2 <= (MATCH_w_32_0 >> 13 & 0x7)) /* c3_16 at 0 */ &&
                        ((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ < 8)) {
                        goto MATCH_label_h0; /*opt-block+*/
                    }
                    else {
                        goto MATCH_label_h2; /*opt-block+*/ /*opt-block+*/
                    }
                }
                else {
                    goto MATCH_label_h1; /*opt-block+*/
                }

                break;

            case 59:

                if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) {
                    if ((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 1) {
                        goto MATCH_label_h2; /*opt-block+*/
                    }
                    else {
                        goto MATCH_label_h0; /*opt-block+*/ /*opt-block+*/
                    }
                }
                else {
                    goto MATCH_label_h1; /*opt-block+*/
                }

                break;

            default:
                assert(0);
            } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/
        }
        goto MATCH_finished_h;

MATCH_label_h0:
        (void)0;   /*placeholder for label*/
        assert(0); /* no match */
        goto MATCH_finished_h;

MATCH_label_h1:
        (void)0; /*placeholder for label*/

#line 106 "machine/hppa/disassembler.m"
        {
            result = 0;
        }

        goto MATCH_finished_h;

MATCH_label_h2:
        (void)0; /*placeholder for label*/

#line 107 "machine/hppa/disassembler.m"
        {
            result = 8;
        }

        goto MATCH_finished_h;

MATCH_finished_h:
        (void)0; /*placeholder for label*/
    }

    // #line 110 "machine/hppa/disassembler.m"
    return result;
}


void NJMCDecoder::dis_c_c(ADDRESS hostpc)
{
    static char *logw[] = { "", ",=", ",<", ",OD", ",TR", ",!=", ",>=", ",EV" };

    /*   static char *logdw[] =  {"",",*=",",*< ",",*OD"
     *                          ,",*TR",",*!=",",*>=",",*EV"}; */
    static char *cmpsubw[] =
    {
        "",    ",=",    ",<",    ",<=",    ",<<",      ",<<=",      ",SV",        ",OD",
        ",TR", ",<>",   ",>=",   ",>",     ",>>=",     ",>>",       ",NSV",       ",EV"
    };
    static char *cmpsubdw[] =
    {
        "",     ",*=",     ",*<",     ",*<=",     ",*<<",       ",*<<=",       ",*SV",         ",*OD",
        ",*TR", ",*<>",    ",*>=",    ",*>",      ",*>>=",      ",*>>",        ",*NSV",        ",*EV"
    };
    /*  static char *bitw[]  = {",<", ",>="};*/
    /*  static char *bitdw[] = {",*<", ",*>="};*/

    // #line 129 "machine/hppa/disassembler.m"
    {
        dword MATCH_p =

#line 129 "machine/hppa/disassembler.m"
            hostpc;
        unsigned MATCH_w_32_0;
        {
            MATCH_w_32_0 = getDword(MATCH_p);

            switch ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */)
            {
            case 0:
            case 1:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 31:
            case 38:
            case 46:
            case 54:
            case 55:
            case 56:
            case 57:
            case 58:
            case 59:
            case 60:
            case 61:
            case 62:
            case 63:
                goto MATCH_label_g0;
                break;

            case 2:

                switch ((MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */)
                {
                case 0:
                case 4:
                case 5:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                case 17:
                case 19:
                case 20:
                case 21:
                case 22:
                case 23:
                case 24:
                case 25:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:
                case 31:
                case 34:
                case 38:
                case 39:
                case 40:
                case 41:
                case 42:
                case 43:
                case 46:
                case 47:
                case 48:
                case 51:
                case 52:
                case 56:
                case 57:
                case 58:
                case 59:
                case 60:

                    if (((MATCH_w_32_0 >> 5 & 0x1) /* d_26 at 0 */ == 1) &&
                        ((0 <= (MATCH_w_32_0 >> 12 & 0x1)) /* f_19 at 0 */ &&
                         ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ < 2))) {
                        unsigned c3_16 = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
                        unsigned neg   = addressToPC(MATCH_p);

#line 135 "machine/hppa/disassembler.m"
                        {
                            apre += sprintf(apre, "%s", cmpsubdw[c3_16 + dis_c_c_n(neg)]);

                            CONS("c_arith_dw ")
                        }
                    } /*opt-block*/ /*opt-block+*/
                    else {
                        goto MATCH_label_g1; /*opt-block+*/
                    }

                    break;

                case 1:
                case 2:
                case 3:
                case 6:
                case 18:
                case 32:
                case 33:
                case 35:
                case 36:
                case 37:
                case 44:
                case 45:
                case 49:
                case 50:
                case 53:
                case 54:
                case 55:
                case 61:
                case 62:
                case 63:
                    goto MATCH_label_g0;
                    break;

                default:
                    assert(0);
                } /* (MATCH_w_32_0 >> 6 & 0x3f) -- ext6_20 at 0 --*/

                break;

            case 10:

#line 158 "machine/hppa/disassembler.m"
                {
                }
                break;

            case 32:
            case 33:
            case 34:
            case 35:
                {
                    unsigned c3_16 = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
                    unsigned neg   = addressToPC(MATCH_p);

#line 139 "machine/hppa/disassembler.m"
                    {
                        apre += sprintf(apre, "%s", cmpsubw[c3_16 + dis_c_c_n(neg)]);

                        CONS("c_cmpb_w ")
                    }
                }
                break;

            case 36:
                goto MATCH_label_g1;
                break;

            case 37:
            case 44:
            case 45:
                goto MATCH_label_g1;
                break;

            case 39:
            case 47:
                {
                    unsigned c3_16 = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
                    unsigned neg   = addressToPC(MATCH_p);

#line 143 "machine/hppa/disassembler.m"
                    {
                        apre += sprintf(apre, "%s", cmpsubdw[c3_16 + dis_c_c_n(neg)]);

                        CONS("c_cmpb_dw ")
                    }
                }
                break;

            case 40:
            case 41:
            case 42:
            case 43:
                goto MATCH_label_g1;
                break;

            case 48:
            case 49:

                if ((MATCH_w_32_0 >> 13 & 0x1) /* d_18 at 0 */ == 1) {
                    unsigned c_16 = (MATCH_w_32_0 >> 15 & 0x1) /* c_16 at 0 */;

#line 155 "machine/hppa/disassembler.m"
                    {
                        apre += sprintf(apre, "[%d]", 1 - c_16);

                        CONS("c_bbs_dw ")
                    }
                } /*opt-block*/ /*opt-block+*/
                else {
                    unsigned c_16 = (MATCH_w_32_0 >> 15 & 0x1) /* c_16 at 0 */;

#line 151 "machine/hppa/disassembler.m"
                    {
                        apre += sprintf(apre, "[%d]", 1 - c_16);

                        CONS("c_bbs_w ")
                    }
                } /*opt-block*/ /*opt-block+*/

                break;

            case 50:
            case 51:
                goto MATCH_label_g2;
                break;

            case 52:

                if (((MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 0) &&
                    ((MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 1)) {
                    goto MATCH_label_g0; /*opt-block+*/
                }
                else {
                    goto MATCH_label_g2; /*opt-block+*/
                }

                break;

            case 53:
                goto MATCH_label_g2;
                break;

            default:
                assert(0);
            } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/
        }
        goto MATCH_finished_g;

MATCH_label_g0:
        (void)0; /*placeholder for label*/

#line 161 "machine/hppa/disassembler.m"
        {
            astr += sprintf(astr, "#c_C%08X", getDword(hostpc));
        }

        goto MATCH_finished_g;

MATCH_label_g1:
        (void)0; /*placeholder for label*/
        {
            unsigned c3_16 = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
            unsigned neg   = addressToPC(MATCH_p);

#line 131 "machine/hppa/disassembler.m"
            {
                apre += sprintf(apre, "%s", cmpsubw[c3_16 + dis_c_c_n(neg)]);

                CONS("c_arith_w ")
            }
        }
        goto MATCH_finished_g;

MATCH_label_g2:
        (void)0; /*placeholder for label*/
        {
            unsigned c3_16 =
                ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ << 2) + (MATCH_w_32_0 >> 13 & 0x3) /* c2_17 at 0 */;

#line 147 "machine/hppa/disassembler.m"
            {
                apre += sprintf(apre, "%s", logw[c3_16]);

                CONS("c_sep ")
            }
        }
        goto MATCH_finished_g;

MATCH_finished_g:
        (void)0; /*placeholder for label*/
    }

    // #line 165 "machine/hppa/disassembler.m"
}


void NJMCDecoder::dis_c_xd(ADDRESS hostpc)
{
    // #line 169 "machine/hppa/disassembler.m"
    {
        dword MATCH_p =

#line 169 "machine/hppa/disassembler.m"
            hostpc;
        unsigned MATCH_w_32_0;
        {
            MATCH_w_32_0 = getDword(MATCH_p);

            switch ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */)
            {
            case 0:
            case 1:
            case 2:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 10:
            case 12:
            case 13:
            case 14:
            case 15:
            case 20:
            case 21:
            case 22:
            case 23:
            case 28:
            case 29:
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
            case 36:
            case 37:
            case 38:
            case 39:
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47:
            case 48:
            case 49:
            case 50:
            case 51:
            case 52:
            case 53:
            case 54:
            case 55:
            case 58:
            case 59:
            case 60:
            case 61:
            case 62:
            case 63:
                goto MATCH_label_f0;
                break;

            case 3:

                if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) {
                    if ((8 <= (MATCH_w_32_0 >> 6 & 0xf)) /* ext4_22 at 0 */ &&
                        ((MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 16)) {
                        unsigned i = (sign_extend((MATCH_w_32_0 & 0x1) /* i_31 at 0 */, 1) << 4) +
                                     (MATCH_w_32_0 >> 1 & 0xf) /* im4_27 at 0 */;

#line 199 "machine/hppa/disassembler.m"
                        {
                            astr += sprintf(astr, "%d", i);

                            CONS("s_addr_r_im ")
                        }
                    } /*opt-block*/ /*opt-block+*/
                    else {
                        goto MATCH_label_f4; /*opt-block+*/ /*opt-block+*/
                    }
                }
                else {
                    switch ((MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */)
                    {
                    case 0:

                        if ((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1) {
                            unsigned x = (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;

#line 175 "machine/hppa/disassembler.m"
                            {
                                apre += sprintf(apre, "%s", ",s");

                                astr += sprintf(astr, "%s", x_11_names[x]);

                                CONS("x_addr_s_byte ")
                            }
                        } /*opt-block*/ /*opt-block+*/
                        else {
                            goto MATCH_label_f1; /*opt-block+*/
                        }

                        break;

                    case 1:

                        if ((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1) {
                            unsigned x = (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;

#line 180 "machine/hppa/disassembler.m"
                            {
                                apre += sprintf(apre, "%s", ",s");

                                astr += sprintf(astr, "%s", x_11_names[x]);

                                CONS("x_addr_s_hwrd ")
                            }
                        } /*opt-block*/ /*opt-block+*/
                        else {
                            goto MATCH_label_f1; /*opt-block+*/
                        }

                        break;

                    case 2:
                    case 6:

                        if ((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1) {
                            goto MATCH_label_f2; /*opt-block+*/
                        }
                        else {
                            goto MATCH_label_f1; /*opt-block+*/
                        }

                        break;

                    case 3:
                    case 4:
                    case 5:
                    case 7:

                        if ((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1) {
                            goto MATCH_label_f3; /*opt-block+*/
                        }
                        else {
                            goto MATCH_label_f1; /*opt-block+*/
                        }

                        break;

                    case 8:
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                        goto MATCH_label_f0;
                        break;

                    default:
                        assert(0);
                    } /* (MATCH_w_32_0 >> 6 & 0xf) -- ext4_22 at 0 --*/
                }

                break;

            case 9:

                if ((MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ == 0) {
                    if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) {
                        goto MATCH_label_f4; /*opt-block+*/
                    }
                    else if (((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1) &&
                             ((0 <= (MATCH_w_32_0 >> 9 & 0x1)) /* addr_22 at 0 */ &&
                              ((MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ < 2))) {
                        goto MATCH_label_f2; /*opt-block+*/
                    }
                    else {
                        goto MATCH_label_f1; /*opt-block+*/ /*opt-block+*/
                    }
                }
                else {
                    goto MATCH_label_f0; /*opt-block+*/
                }

                break;

            case 11:

                if ((MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ == 0) {
                    if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) {
                        goto MATCH_label_f4; /*opt-block+*/
                    }
                    else if (((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1) &&
                             ((0 <= (MATCH_w_32_0 >> 9 & 0x1)) /* addr_22 at 0 */ &&
                              ((MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ < 2))) {
                        goto MATCH_label_f3; /*opt-block+*/
                    }
                    else {
                        goto MATCH_label_f1; /*opt-block+*/ /*opt-block+*/
                    }
                }
                else {
                    goto MATCH_label_f0; /*opt-block+*/
                }

                break;

            case 16:
            case 17:
            case 18:
            case 19:
            case 24:
            case 25:
            case 26:
            case 27:
                {
                    unsigned i = (sign_extend((MATCH_w_32_0 & 0x1) /* i_31 at 0 */, 1) << 13) +
                                 (MATCH_w_32_0 >> 1 & 0x1fff) /* im13_18 at 0 */;

#line 203 "machine/hppa/disassembler.m"
                    {
                        astr += sprintf(astr, "%d", i);

                        CONS("l_addr_16_old ")
                    }
                }
                break;

            case 56:
            case 57:
                {
                    unsigned i = ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 10) +
                                 ((MATCH_w_32_0 >> 16 & 0x1f) /* w5_11 at 0 */ << 11) +
                                 (sign_extend((MATCH_w_32_0 & 0x1) /* w_31 at 0 */, 1) << 16) +
                                 (MATCH_w_32_0 >> 3 & 0x3ff) /* w10_19 at 0 */;

#line 207 "machine/hppa/disassembler.m"
                    {
                        astr += sprintf(astr, "%d", i);

                        CONS("l_addr_17_old ")
                    }
                }
                break;

            default:
                assert(0);
            } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/
        }
        goto MATCH_finished_f;

MATCH_label_f0:
        (void)0; /*placeholder for label*/

#line 211 "machine/hppa/disassembler.m"
        {
            apre += sprintf(apre, "#c_X_ADDR_SHIFT%08X", getDword(hostpc));

            astr += sprintf(astr, "#c_X_ADDR_SHIFT%08X", getDword(hostpc));
        }

        goto MATCH_finished_f;

MATCH_label_f1:
        (void)0; /*placeholder for label*/
        {
            unsigned x = (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;

#line 171 "machine/hppa/disassembler.m"
            {
                astr += sprintf(astr, "%s", x_11_names[x]);

                CONS("x_addr_nots ")
            }
        }
        goto MATCH_finished_f;

MATCH_label_f2:
        (void)0; /*placeholder for label*/
        {
            unsigned x = (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;

#line 185 "machine/hppa/disassembler.m"
            {
                apre += sprintf(apre, "%s", ",s");

                astr += sprintf(astr, "%s", x_11_names[x]);

                CONS("x_addr_s_word ")
            }
        }
        goto MATCH_finished_f;

MATCH_label_f3:
        (void)0; /*placeholder for label*/
        {
            unsigned x = (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;

#line 190 "machine/hppa/disassembler.m"
            {
                apre += sprintf(apre, "%s", ",s");

                astr += sprintf(astr, "%s", x_11_names[x]);

                CONS("x_addr_s_dwrd ")
            }
        }
        goto MATCH_finished_f;

MATCH_label_f4:
        (void)0; /*placeholder for label*/
        {
            unsigned i = (sign_extend((MATCH_w_32_0 >> 16 & 0x1) /* im1_15 at 0 */, 1) << 4) +
                         (MATCH_w_32_0 >> 17 & 0xf) /* im4_11 at 0 */;

#line 195 "machine/hppa/disassembler.m"
            {
                astr += sprintf(astr, "%d", i);

                CONS("s_addr_im_r ")
            }
        }
        goto MATCH_finished_f;

MATCH_finished_f:
        (void)0; /*placeholder for label*/
    }

    // #line 216 "machine/hppa/disassembler.m"
}


void NJMCDecoder::dis_c_wcr(ADDRESS hostpc)
{
    unsigned long regl;

    // #line 220 "machine/hppa/disassembler.m"
    {
        dword MATCH_p =

#line 220 "machine/hppa/disassembler.m"
            hostpc;
        unsigned MATCH_w_32_0;
        {
            MATCH_w_32_0 = getDword(MATCH_p);

            if ((MATCH_w_32_0 >> 5 & 0xff) /* ext8_19 at 0 */ == 69) {
                if ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 0) {
                    if ((MATCH_w_32_0 >> 14 & 0x1) /* ext_17 at 0 */ == 1)

#line 226 "machine/hppa/disassembler.m"
                    {
                        regl = 11;

                        apre += sprintf(apre, ",w");

                        CONS("c_mfctl_w ")
                    }

                    /*opt-block+*/
                    else {
                        unsigned r_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;

#line 222 "machine/hppa/disassembler.m"
                        {
                            regl = r_06;

                            CONS("c_mfctl ")
                        }
                    } /*opt-block*/ /*opt-block+*/ /*opt-block+*/
                }
                else {
                    goto MATCH_label_e0; /*opt-block+*/
                }
            }
            else {
                goto MATCH_label_e0; /*opt-block+*/
            }
        }
        goto MATCH_finished_e;

MATCH_label_e0:
        (void)0; /*placeholder for label*/

#line 231 "machine/hppa/disassembler.m"
        {
            regl = 0;

            apre += sprintf(apre, "#c_WCR%08X#", getDword(hostpc));
        }

        goto MATCH_finished_e;

MATCH_finished_e:
        (void)0; /*placeholder for label*/
    }

    // #line 236 "machine/hppa/disassembler.m"
    astr += sprintf(astr, "%s", cr_06_names[regl]);
}


void NJMCDecoder::dis_c_null(ADDRESS hostpc)
{
    // #line 240 "machine/hppa/disassembler.m"
    {
        dword MATCH_p =

#line 240 "machine/hppa/disassembler.m"
            hostpc;
        unsigned MATCH_w_32_0;
        {
            MATCH_w_32_0 = getDword(MATCH_p);

            switch ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */)
            {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
            case 31:
            case 36:
            case 37:
            case 38:
            case 44:
            case 45:
            case 46:
            case 52:
            case 53:
            case 54:
            case 55:
            case 60:
            case 61:
            case 62:
            case 63:

#line 248 "machine/hppa/disassembler.m"

                apre += sprintf(apre, "#c_NULL%08X#", getDword(hostpc));

                break;

            case 32:
            case 33:
            case 34:
            case 35:
            case 39:
            case 40:
            case 41:
            case 42:
            case 43:
            case 47:
            case 48:
            case 49:
            case 50:
            case 51:
            case 56:
            case 57:
            case 58:
            case 59:

                if ((MATCH_w_32_0 >> 1 & 0x1) /* n_30 at 0 */ == 1)

#line 245 "machine/hppa/disassembler.m"
                {
                    apre += sprintf(apre, ",n");

                    CONS("c_br_null ")
                }

                /*opt-block+*/
                else

#line 242 "machine/hppa/disassembler.m"
                {
                    CONS("c_br_nnull ")
                }

                /*opt-block+*/

                break;

            default:
                assert(0);
            } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/
        }
        goto MATCH_finished_d;

MATCH_finished_d:
        (void)0; /*placeholder for label*/
    }

    // #line 252 "machine/hppa/disassembler.m"
}


void NJMCDecoder::dis_c_bit(ADDRESS hostpc)
{
    // #line 255 "machine/hppa/disassembler.m"
    {
        dword MATCH_p =

#line 255 "machine/hppa/disassembler.m"
            hostpc;
        unsigned MATCH_w_32_0;
        {
            MATCH_w_32_0 = getDword(MATCH_p);

            if ((0 <= (MATCH_w_32_0 >> 26 & 0x3f)) /* op at 0 */ && ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 48)) {
                goto MATCH_label_c0; /*opt-block+*/
            }
            else {
                switch ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */)
                {
                case 50:
                case 51:
                case 52:
                case 53:
                case 54:
                case 55:
                case 56:
                case 57:
                case 58:
                case 59:
                case 60:
                case 61:
                case 62:
                case 63:
                    goto MATCH_label_c0;
                    break;

                case 48:

#line 261 "machine/hppa/disassembler.m"
                    {
                        astr += sprintf(astr, "%s", "%cr11");

                        CONS("c_bitsar ")
                    }
                    break;

                case 49:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* d_18 at 0 */ == 0) {
                        unsigned p_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* p_06 at 0 */;

#line 257 "machine/hppa/disassembler.m"
                        {
                            astr += sprintf(astr, "@%d", p_06);

                            CONS("c_bitpos_w ")
                        }
                    } /*opt-block*/ /*opt-block+*/
                    else {
                        goto MATCH_label_c0; /*opt-block+*/
                    }

                    break;

                default:
                    assert(0);
                } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/
            }
        }
        goto MATCH_finished_c;

MATCH_label_c0:
        (void)0; /*placeholder for label*/

#line 264 "machine/hppa/disassembler.m"

        astr += sprintf(astr, "#c_BIT%08X#", getDword(hostpc));

        goto MATCH_finished_c;

MATCH_finished_c:
        (void)0; /*placeholder for label*/
    }

    // #line 268 "machine/hppa/disassembler.m"
}


void NJMCDecoder::dis_c_addr(ADDRESS hostpc)
{
    // #line 271 "machine/hppa/disassembler.m"
    {
        dword MATCH_p =

#line 271 "machine/hppa/disassembler.m"
            hostpc;
        unsigned MATCH_w_32_0;
        {
            MATCH_w_32_0 = getDword(MATCH_p);

            if ((28 <= (MATCH_w_32_0 >> 26 & 0x3f)) /* op at 0 */ && ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64)) {
                goto MATCH_label_b0; /*opt-block+*/
            }
            else {
                switch ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */)
                {
                case 0:
                case 1:
                case 2:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 10:
                case 12:
                case 13:
                case 14:
                case 15:
                case 20:
                case 21:
                case 22:
                case 23:
                    goto MATCH_label_b0;
                    break;

                case 3:

                    if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) {
                        if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1) {
                            if ((MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1) {
                                if ((12 <= (MATCH_w_32_0 >> 6 & 0xf)) /* ext4_22 at 0 */ &&
                                    ((MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 14))

#line 295 "machine/hppa/disassembler.m"
                                {
                                    apre += sprintf(apre, ",me");

                                    CONS("c_y_addr_me ")
                                }

                                /*opt-block+*/
                                else {
                                    goto MATCH_label_b5; /*opt-block+*/ /*opt-block+*/
                                }
                            }
                            else if ((12 <= (MATCH_w_32_0 >> 6 & 0xf)) /* ext4_22 at 0 */ &&
                                     ((MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 14))

#line 299 "machine/hppa/disassembler.m"
                            {
                                apre += sprintf(apre, ",m");

                                CONS("c_y_addr_m ")
                            }

                            /*opt-block+*/
                            else {
                                goto MATCH_label_b4; /*opt-block+*/ /*opt-block+*/
                            }
                        }
                        else if ((12 <= (MATCH_w_32_0 >> 6 & 0xf)) /* ext4_22 at 0 */ &&
                                 ((MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 14)) {
                            if ((MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1)

#line 291 "machine/hppa/disassembler.m"
                            {
                                apre += sprintf(apre, ",e");

                                CONS("c_y_addr_e ")
                            }

                            /*opt-block+*/
                            else

#line 303 "machine/hppa/disassembler.m"
                            {
                                CONS("c_y_addr_none ")
                            }
                        }

                        /*opt-block+*/ /*opt-block+*/
                        else {
                            goto MATCH_label_b3; /*opt-block+*/
                        }
                    }
                    else if ((0 <= (MATCH_w_32_0 >> 6 & 0xf)) /* ext4_22 at 0 */ &&
                             ((MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 8)) {
                        if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1) {
                            goto MATCH_label_b2; /*opt-block+*/
                        }
                        else {
                            goto MATCH_label_b1; /*opt-block+*/ /*opt-block+*/
                        }
                    }
                    else {
                        goto MATCH_label_b0; /*opt-block+*/
                    }

                    break;

                case 9:
                case 11:

                    if ((MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ == 0) {
                        if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) {
                            if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1) {
                                if (((MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1) &&
                                    ((0 <= (MATCH_w_32_0 >> 9 & 0x1)) /* addr_22 at 0 */ &&
                                     ((MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ < 2))) {
                                    goto MATCH_label_b5; /*opt-block+*/
                                }
                                else {
                                    goto MATCH_label_b4; /*opt-block+*/ /*opt-block+*/
                                }
                            }
                            else {
                                goto MATCH_label_b3; /*opt-block+*/
                            }
                        }
                        else if (((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1) &&
                                 ((0 <= (MATCH_w_32_0 >> 9 & 0x1)) /* addr_22 at 0 */ &&
                                  ((MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ < 2))) {
                            goto MATCH_label_b2; /*opt-block+*/
                        }
                        else {
                            goto MATCH_label_b1; /*opt-block+*/ /*opt-block+*/
                        }
                    }
                    else {
                        goto MATCH_label_b0; /*opt-block+*/
                    }

                    break;

                case 16:
                case 17:
                case 18:
                case 19:
                case 24:
                case 25:
                case 26:
                case 27:

#line 306 "machine/hppa/disassembler.m"
                    {
                        CONS("c_l_addr_none ");
                    }

                    //        | c_addr_s() => {

                    //            apre += sprintf(apre, ",s");

                    //            strcat(constrName, "c_addr_s ");

                    //        }

                    //        | c_addr_m() => {

                    //            apre += sprintf(apre, ",m");

                    //            strcat(constrName, "c_addr_m ");

                    //        }

                    //        | c_addr_sm() => {

                    //            apre += sprintf(apre, ",sm");

                    //            strcat(constrName, "c_addr_sm ");

                    //        }
                    break;

                default:
                    assert(0);
                } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/
            }
        }
        goto MATCH_finished_b;

MATCH_label_b0:
        (void)0; /*placeholder for label*/

#line 323 "machine/hppa/disassembler.m"

        // Do nothing; no completer

        CONS("BUG!!");

        goto MATCH_finished_b;

MATCH_label_b1:
        (void)0; /*placeholder for label*/

#line 288 "machine/hppa/disassembler.m"
        {
            CONS("c_x_addr_notm ")
        }

        goto MATCH_finished_b;

MATCH_label_b2:
        (void)0; /*placeholder for label*/

#line 284 "machine/hppa/disassembler.m"
        {
            apre += sprintf(apre, ",x");

            CONS("c_x_addr_m ")
        }

        goto MATCH_finished_b;

MATCH_label_b3:
        (void)0; /*placeholder for label*/

#line 281 "machine/hppa/disassembler.m"
        {
            CONS("c_s_addr_notm ")
        }

        goto MATCH_finished_b;

MATCH_label_b4:
        (void)0; /*placeholder for label*/

#line 277 "machine/hppa/disassembler.m"
        {
            apre += sprintf(apre, ",ma");

            CONS("c_s_addr_ma ")
        }

        goto MATCH_finished_b;

MATCH_label_b5:
        (void)0; /*placeholder for label*/

#line 273 "machine/hppa/disassembler.m"
        {
            apre += sprintf(apre, ",mb");

            CONS("c_s_addr_mb ")
        }

        goto MATCH_finished_b;

MATCH_finished_b:
        (void)0; /*placeholder for label*/
    }

    // #line 327 "machine/hppa/disassembler.m"
}


void NJMCDecoder::dis_flt_fmt(int fmt)
{
    // Completer for floating point operand size
    switch (fmt)
    {
    case 0:
        apre += sprintf(apre, ",sgl");
        break;

    case 1:
        apre += sprintf(apre, ",dbl");
        break;

    case 3:
        apre += sprintf(apre, ",quad");
        break;

    default:
        apre += sprintf(apre, ",?");
        break;
    }
}


static char regbuf[32];
char *NJMCDecoder::dis_freg(int regNum)
{
    if (regNum >= 32) {
        sprintf(regbuf, "fr%dR", regNum - 32);
    }
    else {
        sprintf(regbuf, "fr%d", regNum);
    }

    return regbuf;
}


