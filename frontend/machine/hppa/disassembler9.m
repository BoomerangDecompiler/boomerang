
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

int NJMCDecoder::decodeAssemblyInstruction (ADDRESS pc, int delta)
{
    char sCmplt[32];
    unsigned long r1,r2;
    sCmplt[0]='\0';
    ADDRESS hostPC = pc + delta;

    astr = _assembly + sprintf(_assembly, "%x: %08x  ", pc, *(unsigned*)hostPC);

    match hostPC to
        | flt_c0(fmt, r, t)[name] => {
            astr += sprintf(astr, "%s", name);
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "  fr%d, fr%d", r, t);
            strcat(constrName, "flt_c0 ");
        }
        | flt_c1(sf, df, r, t)[name] => {
            astr += sprintf(astr, "%s", name);
            dis_flt_fmt(sf);
            dis_flt_fmt(df);
            astr += sprintf(astr, "  fr%d, fr%d", r, t);
            strcat(constrName, "flt_c1 ");
        }
        | flt_c2_0c(r1, r2, c, fmt)[name] => {
            astr += sprintf(astr, "%s", name);
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "  %fr%d, fr%d", r1, r2);
// HACK: Needs completer c decoded
astr += sprintf(astr, "\t\t/* Completer c needs decoding */");
            strcat(constrName, "flt_c2_0c ");
        }
        | flt_c2_0e(r1, r2, c, fmt)[name] => {
            astr += sprintf(astr, "%s", name);
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "  fr%d, fr%d", r1, r2);
// HACK: Needs completer c decoded
astr += sprintf(astr, "\t\t/* Completer c needs decoding */");
            strcat(constrName, "flt_c2_0e ");
        }
        | flt_c3_0c(r1, r2, t, fmt)[name] => {
            astr += sprintf(astr, "%s", name);
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "  fr%d, fr%d, fr%d", r1, r2, t);
            strcat(constrName, "flt_c3_0c ");
        }
        | flt_c3_0e(r1, r2, t, fmt)[name] => {
            astr += sprintf(astr, "%s", name);
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "  fr%d, fr%d, fr%d", r1, r2, t);
            strcat(constrName, "flt_c3_0e ");
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
