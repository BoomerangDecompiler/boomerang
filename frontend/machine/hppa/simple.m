#include "global.h"
#include "decoder.h"

extern char _assembly[];
char* astr;

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


void NJMCDecoder::dis_faddr(ADDRESS faddr)
{
    match faddr to
        | index_faddr (x, s, b) => {
            astr += sprintf(astr, " r%d(sr%d,r%d)", x, s, b);
            strcat(constrName, "index_faddr ");
        }
        | sdisps_faddr(d, s, b) => {
            astr += sprintf(astr, " %d(sr%d,r%d)", d, s, b);
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
        | ins_faddr_x =>
            strcat(constrName, "ins_faddr_x ");
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

void dis_flt_c3(int& d_fmt, int& d_r1, int& d_r2, int& d_t, ADDRESS con)
{
    match con to
        | flt_c3.C(fmt, r1, r2, t)[name] =>
            d_fmt = fmt; d_r1 = r1; d_r2 = r2; d_t = t;
printf("Name of typed constructor is %s\n", name);
        | flt_c3.E(fmt, r1, r2, t)[name] =>
            d_fmt = fmt; d_r1 = r1; d_r2 = r2; d_t = t;
printf("Name of typed constructor is %s\n", name);
        else
            printf("This should never happen\n");
            assert(0);
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
        | floads(c_faddr, faddr, t_27)[name] => {
// Floating point loads and stores
            astr += sprintf(astr, "%s  ", name);
            dis_c_faddr(c_faddr);
            dis_faddr(faddr);
            astr += sprintf(astr, ",fr%d", t_27);
            strcat(constrName, "floads ");
        }
        | fstores(c_faddr, r, faddr)[name] => {
            astr += sprintf(astr, "%s", name);
            dis_c_faddr(c_faddr);
            astr += sprintf(astr, "  fr%d,", r);
            dis_faddr(faddr);
            strcat(constrName, "fstores ");
        }

        | flt_c3(con)[name] => {
            int fmt, r1, r2, t;
            dis_flt_c3(fmt, r1, r2, t, con);
            astr += sprintf(astr, "%s", name);
            dis_flt_fmt(fmt);
            astr += sprintf(astr, "  fr%d, fr%d, fr%d", r1, r2, t);
            strcat(constrName, "flt_c3 ");
        }
        else
            // Do nothing
            sprintf(astr, ".");

    endmatch

    return 4;
}
