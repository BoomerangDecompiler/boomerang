/*
 * Copyright (C) 2000-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:     disassembler.m
 * OVERVIEW: Matcher file for a stand alone disassembler tool
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
 * 20 Aug 01 - Simon: Fixed floating point load/store with ,ma or ,mb
 */



#include "global.h"
#include "decoder.h"
#include "BinaryFile.h"

#include "hppa-names.h"

// globals
extern char _assembly[];
char aprefix[256];
char adata[256];
char* apre;
char* astr;
char *cmpltsep = ".";       // ??

// Prototypes
const char* GetSym(unsigned pc);
const char* GetReloc(unsigned pc);


#define ANAME       apre += sprintf( apre, "%s", name );
//#define APREF(x)    apre += sprintf( apre, "%s", x );
//#define AARGs(x)    astr += sprintf( astr, "%s", x );
//#define AARGd(x)    astr += sprintf( astr, "%d", x );
//#define AARGf(f, x) astr += sprintf( astr, " ## f ## ", x );
//#define Acom        astr += sprintf( astr, "," );
#define CONS(x)     strcat(constrName, x);
#define IGNORE(x)   not_used(*(int*)&x);

// The below is used to quelch annoying "variable not used" warnings
void not_used(int unwanted)
{
    unwanted = 0;
}

DWord getDword (unsigned lc)
/* get4Bytes - returns next 4-Byte from image pointed to by lc.
   Fetch in a big-endian manner  */
{
    return
      (DWord)
      ((((((
          *(Byte *)lc << 8
      ) + *(Byte *)(lc+1)) << 8
      ) + *(Byte *)(lc+2)) << 8
      ) + *(Byte *)(lc+3));
}

static char killBuffer[32];

// Find and kill any dots in the opcode name (and all chars thereafter)
char* killDot(char* str)
{
    strcpy(killBuffer, str);
    char* p = strchr(killBuffer, '.');
    if (p) *p = '\0';
    return killBuffer;
}

static char shexBuffer[32];

char* signedHex(int disp)
{
    if (disp < 0)
        sprintf(shexBuffer, "-0x%x", -disp);
    else
        sprintf(shexBuffer, "0x%x", disp);
    return shexBuffer;
}

// This function returns 0 or 8; the 8 is to access the last half of arrays
// of conditions (e.g. CMPIBF, or the "f" bit of ariths)
int dis_c_c_n(ADDRESS hostpc)
{
    int result = 0;
    match hostpc to
      | c_c_nonneg() => { result = 0; }
      | c_c_neg() =>    { result = 8; }
    endmatch
    return result;
}

void NJMCDecoder::dis_c_c(ADDRESS hostpc)
{
  static char *logw[] =  {"",",=",",<",",OD"
                            ,",TR",",!=",",>=",",EV"};
/*   static char *logdw[] =  {"",",*=",",*< ",",*OD"
                            ,",*TR",",*!=",",*>=",",*EV"}; */
  static char *cmpsubw[] = {"",",=",",<",",<=",",<<",",<<=",
                               ",SV",",OD",
                               ",TR",",<>",",>=",",>",",>>=",
                               ",>>",",NSV",",EV"};
  static char *cmpsubdw[] = {"",",*=",",*<",",*<=",",*<<",",*<<=",
                                ",*SV",",*OD",
                                ",*TR",",*<>",",*>=",",*>",",*>>=",
                                ",*>>",",*NSV",",*EV"};
/*  static char *bitw[]  = {",<", ",>="};*/
/*  static char *bitdw[] = {",*<", ",*>="};*/
  
    match hostpc to
        | c_arith_w(c3_16, neg) => {
            apre += sprintf(apre, "%s", cmpsubw[c3_16 + dis_c_c_n(neg)]);
            CONS("c_arith_w ")
        }
        | c_arith_dw(c3_16, neg) => {
            apre += sprintf(apre, "%s", cmpsubdw[c3_16 + dis_c_c_n(neg)]);
            CONS("c_arith_dw ")
        }
        | c_cmpb_w(c3_16, neg) => {
            apre += sprintf(apre, "%s", cmpsubw[c3_16 + dis_c_c_n(neg)]);
            CONS("c_cmpb_w ")
        }
        | c_cmpb_dw(c3_16, neg) => {
            apre += sprintf(apre, "%s", cmpsubdw[c3_16 + dis_c_c_n(neg)]);
            CONS("c_cmpb_dw ")
        }
        | c_sep(c3_16) => {
            apre += sprintf(apre, "%s", logw[c3_16]);
            CONS("c_sep ")
        }
        | c_bbs_w(c_16) => {
            apre += sprintf(apre, "[%d]",1-c_16);
            CONS("c_bbs_w ")
        }
        | c_bbs_dw(c_16) => {
            apre += sprintf(apre, "[%d]",1-c_16);
            CONS("c_bbs_dw ")
        }
        | c_arith_none() => {
        }
        else {
            astr += sprintf(astr, "#c_C%08X", getDword(hostpc));
        }
  endmatch
}

void NJMCDecoder::dis_c_xd(ADDRESS hostpc)
{

    match hostpc to
        | x_addr_nots(x) =>   {
            astr += sprintf(astr, "%s", x_11_names[x]);
            CONS("x_addr_nots ")
        }
        | x_addr_s_byte(x) => {
            apre += sprintf(apre, "%s", ",s");
            astr += sprintf(astr, "%s", x_11_names[x]);
            CONS("x_addr_s_byte ")
        }
        | x_addr_s_hwrd(x) => {
            apre += sprintf(apre, "%s", ",s");
            astr += sprintf(astr, "%s", x_11_names[x]);
            CONS("x_addr_s_hwrd ")
        }
        | x_addr_s_word(x) => {
            apre += sprintf(apre, "%s", ",s");
            astr += sprintf(astr, "%s", x_11_names[x]);
            CONS("x_addr_s_word ")
        }
        | x_addr_s_dwrd(x) => {
            apre += sprintf(apre, "%s", ",s");
            astr += sprintf(astr, "%s", x_11_names[x]);
            CONS("x_addr_s_dwrd ")
        }
        | s_addr_im_r(i) => {
            astr += sprintf(astr, "%d", i);
            CONS("s_addr_im_r ")
        }
        | s_addr_r_im(i) => {
            astr += sprintf(astr, "%d", i);
            CONS("s_addr_r_im ")
        }
        | l_addr_16_old(i) => {
            astr += sprintf(astr, "%d", i);
            CONS("l_addr_16_old ")
        }
        | l_addr_17_old(i) => {
            astr += sprintf(astr, "%d", i);
            CONS("l_addr_17_old ")
        }
        else {
            apre += sprintf(apre, "#c_X_ADDR_SHIFT%08X", getDword(hostpc));
            astr += sprintf(astr, "#c_X_ADDR_SHIFT%08X", getDword(hostpc));
        }
    endmatch
}

void NJMCDecoder::dis_c_wcr(ADDRESS hostpc)
{
    unsigned long regl;
    match hostpc to
    | c_mfctl(r_06) => {
        regl = r_06;
        CONS("c_mfctl ")
    }
    | c_mfctl_w() => {
        regl = 11;
        apre += sprintf(apre, ",w");
        CONS("c_mfctl_w ")
    }
    else {
        regl = 0;
        apre += sprintf(apre, "#c_WCR%08X#", getDword(hostpc));
    }
  endmatch
  astr += sprintf(astr, "%s", cr_06_names[regl]);
}

void NJMCDecoder::dis_c_null(ADDRESS hostpc)
{
    match hostpc to
        | c_br_nnull() => {
            CONS("c_br_nnull ")
        }
        | c_br_null() => {
            apre += sprintf(apre, ",n");
            CONS("c_br_null ")
        }
        else
            apre += sprintf(apre, "#c_NULL%08X#", getDword(hostpc));
  endmatch
}

void NJMCDecoder::dis_c_bit(ADDRESS hostpc)
{
    match hostpc to
        | c_bitpos_w(p_06) => {
            astr += sprintf(astr, "@%d",p_06);
            CONS("c_bitpos_w ")
        }
        | c_bitsar() => {
            astr += sprintf(astr, "%s", "%cr11");
            CONS("c_bitsar ")
        }
        else
            astr += sprintf(astr, "#c_BIT%08X#", getDword(hostpc));
  endmatch
}

void NJMCDecoder::dis_c_addr(ADDRESS hostpc)
{
    match hostpc to
        | c_s_addr_mb() => {
            apre += sprintf(apre, ",mb");
            CONS("c_s_addr_mb ")
        }
        | c_s_addr_ma() => {
            apre += sprintf(apre, ",ma");
            CONS("c_s_addr_ma ")
        }
        | c_s_addr_notm() => {
            CONS("c_s_addr_notm ")
        }
        | c_x_addr_m() => {
            apre += sprintf(apre, ",x");
            CONS("c_x_addr_m ")
        }
        | c_x_addr_notm() => {
            CONS("c_x_addr_notm ")
        }
        | c_y_addr_e() => {
            apre += sprintf(apre, ",e");
            CONS("c_y_addr_e ")
        }
        | c_y_addr_me() => {
            apre += sprintf(apre, ",me");
            CONS("c_y_addr_me ")
        }
        | c_y_addr_m() => {
            apre += sprintf(apre, ",m");
            CONS("c_y_addr_m ")
        }
        | c_y_addr_none() => {
            CONS("c_y_addr_none ")
        }
        | c_l_addr_none() => {
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

        else
            // Do nothing; no completer
            CONS("BUG!!");
  endmatch
}

void NJMCDecoder::dis_flt_fmt(int fmt)
{
    // Completer for floating point operand size
    switch(fmt) {
        case 0: apre += sprintf(apre, ",sgl"); break;
        case 1: apre += sprintf(apre, ",dbl"); break;
        case 3: apre += sprintf(apre, ",quad"); break;
        default:apre += sprintf(apre, ",?"); break;
    }
}

static char regbuf[32];
char* NJMCDecoder::dis_freg(int regNum)
{
    if (regNum >= 32)
        sprintf(regbuf, "fr%dR", regNum - 32);
    else
        sprintf(regbuf, "fr%d", regNum);
    return regbuf;
}

//void NJMCDecoder::dis_faddr(ADDRESS faddr)
//{
//    match faddr to
//        | index_faddr (x, s, b) => {
//            astr += sprintf(astr, " %s(%s,%s)", b_06_names[x], s2_16_names[s],
//                b_06_names[b]);
//            strcat(constrName, "index_faddr ");
//        }
//        | sdisps_faddr(d, s, b) => {
//            astr += sprintf(astr, " %d(%s,%s)", d, s2_16_names[s],
//                b_06_names[b]);
//            strcat(constrName, "sdisps_faddr ");
//        }
//    endmatch
//}

int NJMCDecoder::decodeAssemblyInstruction (ADDRESS pc, int delta)
{
    ADDRESS hostPC = pc + delta;
    *(apre = aprefix) = '\0';
    *(astr = adata) = '\0';
    apre += sprintf(apre, "%x: %08x  ", pc, *(unsigned*)hostPC);

    match hostPC to
        | NOP => {
            apre += sprintf(apre, "%s", "NOP");
            CONS("NOP ")
        }
        | COPY(r, t) => {
            apre += sprintf(apre, "%s", "COPY");
            astr += sprintf(astr, "%s,%s", r_06_names[r], t_27_names[t]);
            CONS("COPY ")
         }
        | arith(cmplt,r_11,r_06,t_27)[name] => {
            ANAME
            dis_c_c(cmplt);
            astr += sprintf(astr, "%s,%s,%s", r_11_names[r_11],r_06_names[r_06],t_27_names[t_27]);
            CONS("arith ")
        }
        | arith_imm(cmplt, im11_21, r_06, t_11)[name] => {
            ANAME
            dis_c_c(cmplt);
            astr += sprintf(astr, "%d,%s,%s", im11_21, r_06_names[r_06], t_11_names[t_11]);
            CONS("arith_imm ")
        }
        | ADDIL(imm21, r_06)[name] => {
            ANAME
            astr += sprintf(astr, "%d,%s,%s", imm21, r_06_names[r_06], t_11_names[1]);
            CONS("ADDIL ")
        }
        | LDIL(imm21, t_06)[name] => {
            ANAME
            astr += sprintf(astr, "0x%x,%s", imm21, t_06_names[t_06]);
            CONS("LDIL ")
        }
        | iloads(c_addr, xd, s, b,t_27)[name] => {
            ANAME
            dis_c_addr(c_addr);         // Can have ,ma/,mb
            dis_c_xd(xd);
            astr += sprintf(astr, "(%s,%s),%s",
                s2_16_names[s], b_06_names[b],
                t_27_names[t_27]);
            CONS("iloads ")
        }
        | istores(c_addr, r_11, xd, s, b)[name] => {
            ANAME
            dis_c_addr(c_addr);         // Can have ,ma/,mb
            astr += sprintf(astr, "%s,", r_11_names[r_11]);
            dis_c_xd(xd);
            astr += sprintf(astr, "(%s,%s)", s2_16_names[s], b_06_names[b]);
            CONS("istores ")
        }
        | iloads_ldisp(c_addr,xd, s, b,t_11)[name] => {
            ANAME
            IGNORE(c_addr)
            dis_c_xd(xd);
            astr += sprintf(astr, "(%s,%s),%s", 
                s2_16_names[s], b_06_names[b],
                t_11_names[t_11]);
            CONS("iloads_ldisp ")
        }
        | istores_ldisp(c_addr,r_11,xd, s, b)[name] => {
            ANAME
            astr += sprintf(astr, "%s,", r_11_names[r_11]);
            IGNORE(c_addr)
            dis_c_xd(xd);
            astr += sprintf(astr, "(%s,%s)",
                s2_16_names[s], b_06_names[b]);
            CONS("istores_ldisp ")
        }
        | LDO(ldisp, b, t)[name] => {
            ANAME
            astr += sprintf(astr, "%d(%s),%s", ldisp, b_06_names[b], t_06_names[t]);
            CONS("LDO ")
        }
        | VSHD(r1, r2, t, c)[name] => {
            ANAME
            dis_c_c(c);
            astr += sprintf(astr, "%s,%s,%s", r_11_names[r1], r_06_names[r2],
                t_27_names[t]);
            CONS("VSHD ")
        }
        | SHD(r1, r2, p, t, c)[name] => {
            ANAME
            dis_c_c(c);
            astr += sprintf(astr, "%s,%s,%d,%s", r_11_names[r1], r_06_names[r2],
                p, t_27_names[t]);
            CONS("SHD ")
        }
        | ext_var(r, len, t, c)[name] => {
            ANAME
            dis_c_c(c);
            astr += sprintf(astr, "%s,%d,%s", r_11_names[r], len,
                t_27_names[t]);
            CONS("ext_var ")
        }
        | ext_fix(r, p, len, t, c)[name] => {
            ANAME
            dis_c_c(c);
            astr += sprintf(astr, "%s,%d,%d,%s", r_11_names[r], p, len,
                t_27_names[t]);
            CONS("ext_fix ")
        }
        | dep_var(r, len, t, c)[name] => {
            ANAME
            dis_c_c(c);
            astr += sprintf(astr, "%s,%d,%s", r_11_names[r], len,
                t_27_names[t]);
            CONS("dep_var ")
        }
        | dep_fix(r, p, len, t, c)[name] => {
            ANAME
            dis_c_c(c);
            astr += sprintf(astr, "%s,%d,%d,%s", r_11_names[r], p, len,
                t_27_names[t]);
            CONS("dep_fix ")
        }
        | dep_ivar(i, len, t, c)[name] => {
            ANAME
            dis_c_c(c);
            astr += sprintf(astr, "%d,%d,%s", i, len,
                t_27_names[t]);
            CONS("dep_ivar ")
        }
        | dep_ifix(i, p, len, t, c)[name] => {
            ANAME
            dis_c_c(c);
            astr += sprintf(astr, "%d,%d,%d,%s", i, p, len,
                t_27_names[t]);
            CONS("dep_ifix ")
        }
        | ubranch(nulli,ubr_target,t_06)[name] => {
            ANAME
            dis_c_null(nulli);
            // Get the actual destination and its symbol (if possible)
            ADDRESS dest = ubr_target + hostPC - delta;
            const char* dsym = pBF->SymbolByAddress(dest);
            char hexsym[128];
            if (dsym == 0) sprintf(hexsym, "0x%x", dest);
            astr += sprintf(astr, "%s, %s", (dsym ? dsym : hexsym), t_06_names[t_06]);
            CONS("ubranch ")
        }
        | BL.LONG(nulli,ubr_target)[name] => {
            ANAME
            dis_c_null(nulli);
            astr += sprintf(astr, "%d %s", ubr_target, t_06_names[2]);
            CONS("BL.LONG ")
        }
        | BLR(nulli,x_11,t_06)[name] => {
            ANAME
            dis_c_null(nulli);
            astr += sprintf(astr, "%s,%s", x_11_names[x_11], t_06_names[t_06]);
            CONS("BLR ")
        }
        | BV(nulli,x_11,b_06)[name] => {
            ANAME
            dis_c_null(nulli);
            astr += sprintf(astr, "%s(%s)", x_11_names[x_11], b_06_names[b_06]);
            CONS("BV ")
        }
        | bve(p_31,nulli,b_06)[name] => {
            ANAME
            not_used(p_31);
            IGNORE(nulli)
            astr += sprintf(astr, " (%s)", b_06_names[b_06]);
            CONS("bve ")
        /* } */
        /* PA-RISC 2.0 */
        /* | be_all(nulli,cmplt)[name] => { */
        /*    ANAME
            dis_c_null(nulli);
            dis_addr(cmplt);
            CONS("be_all ") */
        }
        | BREAK(im5_27,im13_06)[name] => {
            ANAME
            astr += sprintf(astr, "%d,%d", im5_27,im13_06);
            CONS("BREAK ")
        }
        | sysop_i_t(im10_06,t_27)[name] => {
            ANAME
            astr += sprintf(astr, "%d,%s", im10_06,t_27_names[t_27]);
            CONS("sysop_i_t ");
        }
        | sysop_simple[name] => {
            ANAME
            CONS("sysop_simple ");
        }
        | sysop_r(r_11)[name] => {
            ANAME
            astr += sprintf(astr, "%s", r_11_names[r_11]);
            CONS("sysop_r ")
        }
        | sysop_cr_t(cmplt, t_27)[name] => {
            ANAME
            dis_c_wcr(cmplt);
            astr += sprintf(astr, ",%s", t_27_names[t_27]);
            CONS("sysop_cr_t ")
        }
        | MTCTL(r_11, cr_06)[name] => {
            ANAME
            astr += sprintf(astr, "%s,%s", r_11_names[r_11],cr_06_names[cr_06]);
            CONS("MTCTL ")
        }
        | MFIA(t_27)[name] => {
            ANAME
            astr += sprintf(astr, "%s", t_27_names[t_27]);
            CONS("MFIA ")
        }
        | LDSID(s2_16,b_06,t_27)[name] => {
            ANAME
            astr += sprintf(astr, "(%s,%s),%s", s2_16_names[s2_16],
              b_06_names[b_06], t_27_names[t_27]);
            CONS("LDSID ")
        }
        | MTSP(r_11,sr)[name] => {
            ANAME
            astr += sprintf(astr, "%s,%s", r_11_names[r_11],
              s3_16_names[sr]);
            CONS("MTSP ")
        }
        | MFSP(sr,t_27)[name] => {
            ANAME
            astr += sprintf(astr, "%s,%s", s3_16_names[sr],
              t_27_names[t_27]);
            CONS("MFSP ")
        }
        | addb_all(c_cmplt, null_cmplt, r_11, r_06, target)[name] => {
            ANAME
            dis_c_c(c_cmplt);
            dis_c_null(null_cmplt);
            astr += sprintf(astr, "%s,%s,0x%x", r_11_names[r_11],
              r_06_names[r_06], target + pc + 8);
            CONS("addb_all ")
        }
        | addib_all(c_cmplt, null_cmplt, im5, r_06, target)[name] => {
            ANAME
            dis_c_c(c_cmplt);
            dis_c_null(null_cmplt);
            astr += sprintf(astr, "%d,%s,0x%x", im5,
              r_06_names[r_06], target + pc + 8);
            CONS("addb_all ")
        }
        | cmpb_all(c_cmplt, null_cmplt, r_11, r_06, target)[name] => {
            ANAME
            dis_c_c(c_cmplt);
            dis_c_null(null_cmplt);
            astr += sprintf(astr, "%s,%s,0x%x", r_11_names[r_11],
              r_06_names[r_06], target + pc + 8);
            CONS("cmpb_all ")
        }
        | cmpib_all(c_cmplt, null_cmplt, im5_11, r_06, target)[name] => {
            ANAME
            dis_c_c(c_cmplt);
            dis_c_null(null_cmplt);
            astr += sprintf(astr, "%d,%s,0x%x", im5_11, r_06_names[r_06],
              target + pc + 8);
            CONS("cmpib_all ")
        }
        | bb_all(c_cmplt, null_cmplt, r_11, bit_cmplt, target) => {
            apre += sprintf(apre, "%s", "BB");
            dis_c_c(c_cmplt);
            dis_c_null(null_cmplt);
            astr += sprintf(astr, "%s,", r_11_names[r_11]);
            dis_c_bit(bit_cmplt);
            sprintf(astr, ",%x", target + pc + 8);
            CONS("bb_all ")
        }
        | flt_c0_all(fmt, r, t)[name] => {
            apre += sprintf(apre, "%s", killDot(name));
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "%s, ", dis_freg(r));
            astr += sprintf(astr, "%s",   dis_freg(t));
            CONS("flt_c0_all ")
        }
        | flt_c1_all(sf, df, r, t)[name] => {
            apre += sprintf(apre, "%s", killDot(name));
            dis_flt_fmt(sf);
            dis_flt_fmt(df);
            astr += sprintf(astr, "%s, ", dis_freg(r));
            astr += sprintf(astr, "%s",   dis_freg(t));
            CONS("flt_c1_all ")
        }
        | flt_c2_all(fmt, c, r1, r2)[name] => {
            apre += sprintf(apre, "%s", killDot(name));
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "%s, ", dis_freg(r1));
            astr += sprintf(astr, "%s",   dis_freg(r2));
// HACK: Needs completer c decoded
astr += sprintf(astr, "\t/* Completer c needs decoding */");
IGNORE(c)
            CONS("flt_c2_all ")
        }
        | flt_c3_all(fmt, r1, r2, t)[name] => {
            apre += sprintf(apre, "%s", killDot(name));
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "%s, ", dis_freg(r1));
            astr += sprintf(astr, "%s, ", dis_freg(r2));
            astr += sprintf(astr, "%s",   dis_freg(t));
            CONS("flt_c3_all ")
        }
        | XMPYU(r1, r2, t) => {
            apre += sprintf(apre, "XMPYU");
            astr += sprintf(astr, "%s, ", dis_freg(r1));
            astr += sprintf(astr, "%s, ", dis_freg(r2));
            astr += sprintf(astr, "%s",   dis_freg(t));
            CONS("FMPYU ");
        }

// Floating point loads and stores
        | fwloads(c_addr, xd, s, b, t_27)[name] => {
            apre += sprintf(apre, "%s",killDot(name));
//          dis_c_addr(c_faddr);
            dis_c_addr(c_addr);
            dis_c_xd(xd);
            astr += sprintf(astr, "(%s,%s),%s",
                s2_16_names[s], b_06_names[b],
                dis_freg(t_27));
            CONS("fwloads ")
        }
        | fwstores(c_addr, r, xd, s, b)[name] => {
            apre += sprintf(apre, "%s",killDot(name));
//          dis_c_addr(c_faddr);
            dis_c_addr(c_addr);
            astr += sprintf(astr, "%s,", dis_freg(r));
            dis_c_xd(xd);
            astr += sprintf(astr, "(%s,%s)",
                s2_16_names[s], b_06_names[b]);
            CONS("fwstores ")
        }
        | fdloads(c_addr, xd, s, b, t_27)[name] => {
            apre += sprintf(apre, "%s",killDot(name));
//          dis_c_addr(c_faddr);
            dis_c_addr(c_addr);
            dis_c_xd(xd);
            astr += sprintf(astr, "(%s,%s),%s",
                s2_16_names[s], b_06_names[b],
                dis_freg(t_27));
            CONS("fdloads ")
        }
        | fdstores(c_addr, r, xd, s, b)[name] => {
            apre += sprintf(apre, "%s", killDot(name));
//          dis_c_addr(c_faddr);
            dis_c_addr(c_addr);
            astr += sprintf(astr, "%s,",   dis_freg(r));
            dis_c_xd(xd);
            astr += sprintf(astr, "(%s,%s)",
                s2_16_names[s], b_06_names[b]);
            CONS("fdstores ")
        }

        else
            apre += sprintf(apre, "unrecog. %02X %08X",
              (getDword(hostPC) >> 26) & 0x3F, getDword(hostPC) & 0x03FFFFFF);
    endmatch
    // Output the two pieces, and make an attempt to have the second column
    // line up (but not have too big a gap between them). 28 seems OK
    sprintf(_assembly, "%s%*s%s", aprefix, 28-strlen(aprefix), " ", adata);
    return 4;               // Always advance 4 bytes, even for unrecog etc

}

/*
                }
                | LDWl(cmplt, ldisp, s2_16, b_06, t_11)[name] => {
                        astr += sprintf(astr, "%s", name);
                        c_disps(cmplt);
                        astr += sprintf(astr, "  %d(%s,%s),%s", ldisp, s2_16_names[s2_16], b_06_names[b_06], t_11_names[t_11]);
                
*/
