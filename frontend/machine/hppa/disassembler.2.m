/*==============================================================================
 * FILE:     disassembler.m
 * OVERVIEW: Matcher file for a stand alone disassembler tool
 *
 (C) 2000-2001 The University of Queensland, BT group
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
 */


/*
  void NJMCDecoder :: Cmplt_1(ADDRESS hostpc1)
  {
  }

  void NJMCDecoder :: Cmplt_2(ADDRESS hostpc2)
  {
  }

  void NJMCDecoder :: Cmplt_3(ADDRESS hostpc3)
  {
  }
 */


#include "global.h"
#include "decoder.h"
#include "BinaryFile.h"

#include "hppa-names.h"

// globals
extern char _assembly[];
char* astr;
char *cmpltsep = ".";       // ??

// Prototypes
const char* GetSym(unsigned pc);
const char* GetReloc(unsigned pc);


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


void NJMCDecoder::dis_c_c(ADDRESS hostpc)
{
  static char *logw[] =  {"",",=",",<",",OD"
                            ,",TR",",!=",",>=",",EV"};
  static char *logdw[] =  {"",",*=",",*<] ",",*OD"
                            ,",*TR",",*!=",",*>=",",*EV"};
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
        | c_arith_w(c3_16) => {
            astr += sprintf(astr, "%s", logw[c3_16]);
            strcat(constrName, "c_arith_w ");
        }
        | c_arith_dw(c3_16) => {
            astr += sprintf(astr, "%s", logdw[c3_16]);
            strcat(constrName, "c_arith_dw ");
        }
        | c_cmpb_w(c3_16) => {
            astr += sprintf(astr, "%s", cmpsubw[c3_16]);
            strcat(constrName, "c_cmpb_w ");
        }
        | c_cmpb_dw(c3_16) => {
            astr += sprintf(astr, "%s", cmpsubdw[c3_16]);
            strcat(constrName, "c_cmpb_dw ");
        }
        | c_bbs_w(c_16) => {
            astr += sprintf(astr, "[%d]",1-c_16);
            strcat(constrName, "c_bbs_w ");
        }
        | c_bbs_dw(c_16) => {
            astr += sprintf(astr, "[%d]",1-c_16);
            strcat(constrName, "c_bbs_dw ");
        }
        | c_arith_none() => {
        }
        else {
            astr += sprintf(astr, "#c_C%08X", getDword(hostpc));
        }
  endmatch
}

void NJMCDecoder::dis_addr(ADDRESS hostpc)
{
    match hostpc to
        | index_addr(x, ss, b, cmplt) => {
            astr += sprintf(astr, "%s(%s,%s)",b_06_names[x], s2_16_names[ss],
              b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "index_addr ");
        }
        | indexabs_addr(x, b, cmplt) => {
            astr += sprintf(astr, "%s(%s)",b_06_names[x], b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "indexabs_addr ");
        }
        | sdispl_addr(im5, ss, b, cmplt) => {
            astr += sprintf(astr, "%d(%s,%s)", im5, s2_16_names[ss],
              b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "sdispl_addr ");
        }
        | sdisplabs_addr(im5, b, cmplt) => {
            astr += sprintf(astr, "%d(%s)", im5, b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "sdisplabs_addr ");
        }
        | sdisps_addr(im5, ss, b, cmplt) => {
            astr += sprintf(astr, "%d(%s,%s)", im5, s2_16_names[ss],
              b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "sdisps_addr ");
        }
        | sdispsabs_addr(im5, b, cmplt) => {
            astr += sprintf(astr, "%d(%s)", im5, b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "sdispsabs_addr ");
        }
        | ldispa12_addr(ldisp, ss, b, cmplt) => {
            astr += sprintf(astr, "%d(%s,%s)", ldisp, s2_16_names[ss],
              b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "ldispa12_addr ");
        }
        | ldispa16_addr(ldisp, ss, b, cmplt) => {
            astr += sprintf(astr, "%d(%s,%s)", ldisp, s2_16_names[ss],
              b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "ldispa16_addr ");
        }
        | ldispa16ma_addr(ldisp, ss, b, cmplt) => {
            astr += sprintf(astr, "%d(%s,%s)", ldisp, s2_16_names[ss],
              b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "ldispa16ma_addr ");
        }
        | ldispa16mb_addr(ldisp, ss, b, cmplt) => {
            astr += sprintf(astr, "%d(%s,%s)", ldisp, s2_16_names[ss],
              b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "ldispa16mb_addr ");
        }
        | ldispa16abs_addr(ldisp, b, cmplt) => {
            astr += sprintf(astr, "%d(%s)", ldisp, b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "ldispa16abs_addr ");
        }
        | ldispa16a_addr(ldisp, ss, b, cmplt) => {
            astr += sprintf(astr, "%d(%s,%s)", ldisp, s2_16_names[ss],
              b_06_names[b]);
            dis_c_addr(cmplt);
            strcat(constrName, "ldispa16a_addr ");
        }
        | bea17_addr(offset, ss, b) => {
            astr += sprintf(astr, "%d(%s,%s)", offset, s3_16_names[ss],
              b_06_names[b]);
            strcat(constrName, "bea17_addr ");
        }
        else {
            astr += sprintf(astr, "#ADDR%08X",getDword(hostpc));
        }
  endmatch
}

unsigned long NJMCDecoder::dis_c_wcr(ADDRESS hostpc)
{
    unsigned long regl;
    match hostpc to
    | c_mfctl(r_06) => {
        //astr += sprintf(astr, "");
        regl = r_06;
        strcat(constrName, "c_mfctl ");
    }
    | c_mfctl_w() => {
        astr += sprintf(astr, ".w");
        regl = 11;
        strcat(constrName, "c_mfctl_w ");
    }
    else {
        regl = 0;
        astr += sprintf(astr, "#c_WCR%08X#", getDword(hostpc));
    }
  endmatch
  return regl;
}

void NJMCDecoder::dis_c_null(ADDRESS hostpc)
{
    match hostpc to
        | c_br_nnull() => {
            //astr += sprintf(astr, "");
            strcat(constrName, "c_br_nnull ");
        }
        | c_br_null() => {
            astr += sprintf(astr, ",n");
            strcat(constrName, "c_br_null ");
        }
        else
            astr += sprintf(astr, "#c_NULL%08X#", getDword(hostpc));
  endmatch
}

void NJMCDecoder::dis_c_bit(ADDRESS hostpc)
{
    match hostpc to
        | c_bitpos_w(p_06) => {
            astr += sprintf(astr, "@%d",p_06);
            strcat(constrName, "c_bitpos_w ");
        }
        | c_bitpos_dw(p_06) => {
            astr += sprintf(astr, "@%d",p_06 + 32);
            strcat(constrName, "c_bitpos_dw ");
        }
        | c_bitsar() => {
            astr += sprintf(astr, "%s", "%cr11");
            strcat(constrName, "c_bitsar ");
        }
        else
            astr += sprintf(astr, "#c_BIT%08X#", getDword(hostpc));
  endmatch
}

void NJMCDecoder::dis_c_addr(ADDRESS hostpc)
{
    match hostpc to
        | ins_b => {
            astr += sprintf(astr, "[b]");
            strcat(constrName, "ins_b ");
        }
        | ins_bm => {
            astr += sprintf(astr, "[bm]");
            strcat(constrName, "ins_bm ");
        }
        | ins_e => {
            astr += sprintf(astr, "[e]");
            strcat(constrName, "ins_e ");
        }
        | ins_em => {
            astr += sprintf(astr, "[em]");
            strcat(constrName, "ins_em ");
        }
        | ins_def_d => {
            //astr += sprintf(astr, "");
            strcat(constrName, "ins_def_d ");
        }
        | ins_ma => {
            astr += sprintf(astr, "[ma]");
            strcat(constrName, "ins_ma ");
        }
        | ins_mb => {
            astr += sprintf(astr, "[mb]");
            strcat(constrName, "ins_mb ");
        }
        | ins_o => {
            astr += sprintf(astr, "[o]");
            strcat(constrName, "ins_o ");
        }
        | ins_def_il => {
            //astr += sprintf(astr, "");
            strcat(constrName, "ins_def_il ");
        }
        | ins_s => {
            astr += sprintf(astr, "[s]");
            strcat(constrName, "ins_s ");
        }
        | ins_sm => {
            astr += sprintf(astr, "[sm]");
            strcat(constrName, "ins_sm ");
        }
        | ins_m => {
            astr += sprintf(astr, "[m]");
            strcat(constrName, "ins_m ");
        }
        else
            astr += sprintf(astr, "#c_addr%08X#", getDword(hostpc));
  endmatch
}

void NJMCDecoder::dis_flt_fmt(int fmt)
{
    // Completer for floating point operand size
    switch(fmt) {
        case 0: astr += sprintf(astr, ",sgl"); break;
        case 1: astr += sprintf(astr, ",dbl"); break;
        case 3: astr += sprintf(astr, ",quad"); break;
        default:astr += sprintf(astr, ",?"); break;
    }
}

void NJMCDecoder::dis_faddr(ADDRESS faddr)
{
    match faddr to
        | index_faddr (x, s, b) => {
            astr += sprintf(astr, " %s(%s,%s)", b_06_names[x], s2_16_names[s],
                b_06_names[b]);
            strcat(constrName, "index_faddr ");
        }
        | sdisps_faddr(d, s, b) => {
            astr += sprintf(astr, " %d(%s,%s)", d, s2_16_names[s],
                b_06_names[b]);
            strcat(constrName, "sdisps_faddr ");
        }
    endmatch
}

void NJMCDecoder::dis_c_faddr(ADDRESS c_faddr)
{
    match c_faddr to
        | ins_faddr_s  => {
            astr += sprintf(astr, ",s");
            strcat(constrName, "ins_faddr_s ");
        }
        | ins_faddr_m  => {
            astr += sprintf(astr, ",m");
            strcat(constrName, "ins_faddr_m ");
        }
        | ins_faddr_sm => {
            astr += sprintf(astr, ",sm");
            strcat(constrName, "ins_faddr_sm ");
        }
        | ins_faddr_x => {
            strcat(constrName, "ins_faddr_x ");
        }
        | ins_faddr_mb => {
            astr += sprintf(astr, ",mb");
            strcat(constrName, "ins_faddr_mb ");
        }
        | ins_faddr_ma => {
            astr += sprintf(astr, ",ma");
            strcat(constrName, "ins_faddr_ma ");
        }
        | ins_faddr_si => {
            strcat(constrName, "ins_faddr_si ");
        }
    endmatch
}

int NJMCDecoder::decodeAssemblyInstruction (ADDRESS pc, int delta)
{
    char sCmplt[32];
    unsigned long r1,r2;
    sCmplt[0]='\0';
    ADDRESS hostPC = pc + delta;

    astr = _assembly + sprintf(_assembly, "%x: %08x  ", pc, *(unsigned*)hostPC);

    match hostPC to
        | NOP => {
            astr += sprintf(astr, "NOP");
        }
        | COPY(r, t) => {
            astr += sprintf(astr, "COPY %s,%s", r_06_names[r], t_27_names[t]);
        }
        | arith(cmplt,r_11,r_06,t_27)[name] => {
            /*  Arith,cc3_16   r_11, r_06, t_27 */
            astr += sprintf(astr, "%s", name);
            dis_c_c(cmplt);
            astr += sprintf(astr, "  %s,%s,%s",
                r_11_names[r_11], r_06_names[r_06], t_27_names[t_27]);
            strcat(constrName, "arith ");
        }
        | arith_imm(cmplt, im11_21, r_06, t_11)[name] => {
            /* arith_imm,cc3_16 im11_21!,r_06,t_11 */
            astr += sprintf(astr, "%s", name);
            dis_c_c(cmplt);
            astr += sprintf(astr, "  %d,%s,%s", im11_21, r_06_names[r_06],
                t_11_names[t_11]);
            strcat(constrName, "arith_imm ");
        }
        | ADDIL(imm21, r_06)[name] => {
            astr += sprintf(astr, "%s", name);
            /* dis_c_c(cmplt); */
            astr += sprintf(astr, "  %d,%s,%s", imm21, r_06_names[r_06],
              t_11_names[1]);
            strcat(constrName, "ADDIL ");
        }
        | LDIL(imm21, t_06)[name] => {
            astr += sprintf(astr, "%s", name);
            /* dis_c_c(cmplt); */
            astr += sprintf(astr, "  %d,%s", imm21, t_06_names[t_06]);
            strcat(constrName, "LDIL ");
        }
        | iloads(addr,t_27)[name] => {
            astr += sprintf(astr, "%s  ", name);
            dis_addr(addr);
            astr += sprintf(astr, ",%s",t_27_names[t_27]);
            strcat(constrName, "iloads ");
        }
        | istores(r_11,addr)[name] => {
            astr += sprintf(astr, "%s", name);
            astr += sprintf(astr, "  %s", r_11_names[r_11]);
            dis_addr(addr);
            strcat(constrName, "istores ");
        }
        | ubranch(nulli,ubr_target,t_06)[name] => {
            /* ubranch,cmplt,n  target,t_06) */
            astr += sprintf(astr, "%s", name);
            dis_c_null(nulli);
            // Get the actual destination
            ADDRESS dest = ubr_target + hostPC - delta;
            // Get a symbol for it, if possible
            const char* dsym = pBF->SymbolByAddress(dest);
            char hexsym[128];
            if (dsym == 0)
                 sprintf(hexsym, "0x%x", dest);
            astr += sprintf(astr, " %s, %s", (dsym ? dsym : hexsym),
              t_06_names[t_06]);
            strcat(constrName, "ubranch ");
        }
        | BL.LONG(nulli,ubr_target)[name] => {
            /* BL.LONG cmplt,n  target,2) */
            astr += sprintf(astr, "%s", name);
            dis_c_null(nulli);
            astr += sprintf(astr, " %d %s", ubr_target, t_06_names[2]);
            strcat(constrName, "BL.LONG ");
        }
        | BLR(nulli,x_11,t_06)[name] => {
            /* BLR,n x,t */
            astr += sprintf(astr, "%s", name);
            dis_c_null(nulli);
            astr += sprintf(astr, "  %s,%s", x_11_names[x_11], t_06_names[t_06]);
            strcat(constrName, "BLR ");
        }
        | BV(nulli,x_11,b_06)[name] => {
            /* BV,n x_11(b_06) */
            astr += sprintf(astr, "%s", name);
            dis_c_null(nulli);
            astr += sprintf(astr, "  %s(%s)", x_11_names[x_11], b_06_names[b_06]);
            strcat(constrName, "BV ");
        }
        | bve(p_31,nulli,b_06)[name] => {
            /* BVE.l BVE.lp BVE.p BVE  */
            astr += sprintf(astr, "%s", name);
            dis_c_null(nulli);
            astr += sprintf(astr, " (%s)", b_06_names[b_06]);
            strcat(constrName, "bve ");
        }
        | be_all(nulli,cmplt)[name] => {
            astr += sprintf(astr, "%s", name);
            dis_c_null(nulli);
            astr += sprintf(astr, "  ");
            dis_addr(cmplt);
            strcat(constrName, "be_all ");
        }
        | loads_l(cmplt,t_11)[name] => {
            astr += sprintf(astr, "%s  ", name);
            dis_addr(cmplt);
            astr += sprintf(astr, ",%s", t_11_names[t_11]);
            strcat(constrName, "loads_l ");
        }
        | stores_l(r_11,cmplt)[name] => {
            astr += sprintf(astr, "%s  %s,", name, r_11_names[r_11]);
            dis_addr(cmplt);
            strcat(constrName, "stores_l ");
        }
        | BREAK(im5_27,im13_06)[name] => {
            astr += sprintf(astr, "%s  %d,%d",name, im5_27,im13_06);
            strcat(constrName, "BREAK ");
        }
        | sysop_i_t(im10_06,t_27)[name] => {
            astr += sprintf(astr, "%s  %d,%s",name, im10_06,t_27_names[t_27]);
            strcat(constrName, "sysop_i_t ");
        }
        | sysop_simple[name] => {
            astr += sprintf(astr, "%s", name);
            strcat(constrName, "sysop_simple ");
        }
        | sysop_r(r_11)[name] => {
            astr += sprintf(astr, "%s  %s", name, r_11_names[r_11]);
            strcat(constrName, "sysop_r ");
        }
        | sysop_cr_t(cmplt, t_27)[name] => {
            astr += sprintf(astr, "%s", name);
            r1 = dis_c_wcr(cmplt);
            astr += sprintf(astr, "  %s,%s", cr_06_names[r1], t_27_names[t_27]);
            strcat(constrName, "sysop_cr_t ");
        }
        | MTCTL(r_11, ct_06)[name] => {
            astr += sprintf(astr, "%s  %s", name, ct_06_names[ct_06]);
            // Note: r_11 not used!
            strcat(constrName, "MTCTL ");
        }
        | MFIA(t_27)[name] => {
            astr += sprintf(astr, "%s  %s", name, t_27_names[t_27]);
            strcat(constrName, "MFIA ");
        }
        | LDSID(s2_16,b_06,t_27)[name] => {
            astr += sprintf(astr, "%s  (%s,%s),%s", name, s2_16_names[s2_16],
              b_06_names[b_06], t_27_names[t_27]);
            strcat(constrName, "LDSID ");
        }
        | MTSP(r_11,sr)[name] => {
            astr += sprintf(astr, "%s  %s,%s", name, r_11_names[r_11],
              s3_16_names[sr]);
            strcat(constrName, "MTSP ");
        }
        | MFSP(sr,t_27)[name] => {
            astr += sprintf(astr, "%s  %s,%s", name, s3_16_names[sr],
              t_27_names[t_27]);
            strcat(constrName, "MFSP ");
        }
        | cmpb_all(c_cmplt, null_cmplt, r_11, r_06, target)[name] => {
            astr += sprintf(astr, "%s", name);
            dis_c_c(c_cmplt);
            dis_c_null(null_cmplt);
            astr += sprintf(astr, "  %s,%s,0x%x", r_11_names[r_11],
              r_06_names[r_06], target + pc + 8);
            strcat(constrName, "cmpb_all ");
        }
        | cmpib_all(c_cmplt, null_cmplt, im5_11, r_06, target)[name] => {
            astr += sprintf(astr, "%s", name);
            dis_c_c(c_cmplt);
            dis_c_null(null_cmplt);
            astr += sprintf(astr, "  %d,%s,0x%x", im5_11, r_06_names[r_06],
              target + pc + 8);
            strcat(constrName, "cmpib_all ");
        }
        | bb_all(c_cmplt, null_cmplt, r_11, bit_cmplt, target) => {
            astr += sprintf(astr, "%s", "BB");
            dis_c_c(c_cmplt);
            dis_c_null(null_cmplt);
            astr += sprintf(astr, "  %s,", r_11_names[r_11]);
            dis_c_bit(bit_cmplt);
            sprint(astr, ",%x", target + pc + 8);
            strcat(constrName, "bb_all ");
        }
        | flt_c0_all(fmt, r, t)[name] => {
            astr += sprintf(astr, "%s", killDot(name));
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "  fr%d, fr%d", r, t);
            strcat(constrName, "flt_c0_all ");
        }
        | flt_c1_all(sf, df, r, t)[name] => {
            astr += sprintf(astr, "%s", killDot(name));
            dis_flt_fmt(sf);
            dis_flt_fmt(df);
            astr += sprintf(astr, "  fr%d, fr%d", r, t);
            strcat(constrName, "flt_c1_all ");
        }
        | flt_c2_all(fmt, c, r1, r2)[name] => {
            astr += sprintf(astr, "%s", killDot(name));
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "  fr%d, fr%d", r1, r2);
// HACK: Needs completer c decoded
astr += sprintf(astr, "\t/* Completer c needs decoding */");
            strcat(constrName, "flt_c2_all ");
        }
        | flt_c3_all(fmt, r1, r2, t)[name] => {
            astr += sprintf(astr, "%s", killDot(name));
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "  fr%d, fr%d, fr%d", r1, r2, t);
            strcat(constrName, "flt_c3_all ");
        }

// Floating point loads and stores
        | floads(c_faddr, faddr, t_27)[name] => {
            astr += sprintf(astr, "%s", killDot(name));
            dis_c_faddr(c_faddr);
            astr += sprintf(astr, "  ");
            dis_faddr(faddr);
            astr += sprintf(astr, ",fr%d",t_27);
            strcat(constrName, "floads ");
        }
        | fstores(c_faddr, r, faddr)[name] => {
            astr += sprintf(astr, "%s", killDot(name));
            dis_c_faddr(c_faddr);
            astr += sprintf(astr, "  fr%d", r);
            dis_faddr(faddr);
            strcat(constrName, "fstores ");
        }

        else
            astr += sprintf(astr, "unrecog. %02X %08X",
              (getDword(hostPC) >> 26) & 0x3F, getDword(hostPC) & 0x03FFFFFF);
    endmatch

    return 4;               // Always advance 4 bytes, even for unrecog etc

}

/*
                }
                | LDWl(cmplt, ldisp, s2_16, b_06, t_11)[name] => {
                        astr += sprintf(astr, "%s", name);
                        c_disps(cmplt);
                        astr += sprintf(astr, "  %d(%s,%s),%s", ldisp, s2_16_names[s2_16], b_06_names[b_06], t_11_names[t_11]);
                
*/
