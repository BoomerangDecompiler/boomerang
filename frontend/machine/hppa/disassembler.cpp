#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 2 "machine/hppa/disassembler.m"
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

DWord getDword (ADDRESS lc)
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


#line 105 "machine/hppa/disassembler.m"
{ 
  dword MATCH_p = 
    
    #line 105 "machine/hppa/disassembler.m"
    hostpc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    
      switch((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */) {
        case 0: case 1: case 3: case 4: case 5: case 6: case 7: case 8: 
        case 9: case 10: case 11: case 12: case 13: case 14: case 15: 
        case 16: case 17: case 18: case 19: case 20: case 21: case 22: 
        case 23: case 24: case 25: case 26: case 27: case 28: case 29: 
        case 30: case 31: case 38: case 46: case 48: case 49: case 54: 
        case 55: case 56: case 57: case 58: case 60: case 61: case 62: 
        case 63: 
          goto MATCH_label_h0; break;
        case 2: 
          
            switch((MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */) {
              case 0: case 4: case 5: case 7: case 8: case 9: case 10: 
              case 11: case 12: case 13: case 14: case 15: case 16: case 17: 
              case 19: case 20: case 21: case 22: case 23: case 24: case 25: 
              case 26: case 27: case 28: case 29: case 30: case 31: case 34: 
              case 38: case 39: case 40: case 41: case 42: case 43: case 46: 
              case 47: case 48: case 51: case 52: case 56: case 57: case 58: 
              case 59: case 60: 
                if ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ == 1) 
                  goto MATCH_label_h2;  /*opt-block+*/
                else 
                  goto MATCH_label_h1;  /*opt-block+*/
                
                break;
              case 1: case 2: case 3: case 6: case 18: case 32: case 33: 
              case 35: case 36: case 37: case 44: case 45: case 49: case 50: 
              case 53: case 54: case 55: case 61: case 62: case 63: 
                goto MATCH_label_h0; break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 6 & 0x3f) -- ext6_20 at 0 --*/ 
          break;
        case 32: case 33: case 39: case 40: case 41: 
          goto MATCH_label_h1; break;
        case 34: case 35: case 42: case 43: case 47: 
          goto MATCH_label_h2; break;
        case 36: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ == 1) 
            goto MATCH_label_h2;  /*opt-block+*/
          else 
            goto MATCH_label_h1;  /*opt-block+*/
          
          break;
        case 37: case 44: case 45: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ == 1 && 
            (0 <= (MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ && 
            (MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ < 2)) 
            goto MATCH_label_h2;  /*opt-block+*/
          else 
            goto MATCH_label_h1;  /*opt-block+*/
          
          break;
        case 50: case 51: 
          if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) 
            goto MATCH_label_h2;  /*opt-block+*/
          else 
            goto MATCH_label_h1;  /*opt-block+*/
          
          break;
        case 52: 
          if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 0 || 
              2 <= (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ && 
              (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ < 8 || 
              (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 1 && 
              (MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 0 && 
              (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 1) 
              goto MATCH_label_h0;  /*opt-block+*/
            else 
              goto MATCH_label_h2;  /*opt-block+*/ /*opt-block+*/
          else 
            if ((MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 0 && 
              (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 1) 
              goto MATCH_label_h0;  /*opt-block+*/
            else 
              goto MATCH_label_h1;  /*opt-block+*/ /*opt-block+*/
          break;
        case 53: 
          if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 0 || 
              2 <= (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ && 
              (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ < 8) 
              goto MATCH_label_h0;  /*opt-block+*/
            else 
              goto MATCH_label_h2;  /*opt-block+*/ /*opt-block+*/
          else 
            goto MATCH_label_h1;  /*opt-block+*/
          break;
        case 59: 
          if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 1) 
              goto MATCH_label_h2;  /*opt-block+*/
            else 
              goto MATCH_label_h0;  /*opt-block+*/ /*opt-block+*/
          else 
            goto MATCH_label_h1;  /*opt-block+*/
          break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_h; 
  
  MATCH_label_h0: (void)0; /*placeholder for label*/ 
    assert(0);  /* no match */ 
    goto MATCH_finished_h; 
    
  MATCH_label_h1: (void)0; /*placeholder for label*/ 
    
    #line 106 "machine/hppa/disassembler.m"
     { result = 0; }

    
     
    goto MATCH_finished_h; 
    
  MATCH_label_h2: (void)0; /*placeholder for label*/ 
    
    #line 107 "machine/hppa/disassembler.m"
        { result = 8; }

    
     
    goto MATCH_finished_h; 
    
  MATCH_finished_h: (void)0; /*placeholder for label*/
  
}

#line 110 "machine/hppa/disassembler.m"
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
  


#line 129 "machine/hppa/disassembler.m"
{ 
  dword MATCH_p = 
    
    #line 129 "machine/hppa/disassembler.m"
    hostpc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    
      switch((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */) {
        case 0: case 1: case 3: case 4: case 5: case 6: case 7: case 8: 
        case 9: case 11: case 12: case 13: case 14: case 15: case 16: 
        case 17: case 18: case 19: case 20: case 21: case 22: case 23: 
        case 24: case 25: case 26: case 27: case 28: case 29: case 30: 
        case 31: case 38: case 46: case 54: case 55: case 56: case 57: 
        case 58: case 59: case 60: case 61: case 62: case 63: 
          goto MATCH_label_g0; break;
        case 2: 
          
            switch((MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */) {
              case 0: case 4: case 5: case 7: case 8: case 9: case 10: 
              case 11: case 12: case 13: case 14: case 15: case 16: case 17: 
              case 19: case 20: case 21: case 22: case 23: case 24: case 25: 
              case 26: case 27: case 28: case 29: case 30: case 31: case 34: 
              case 38: case 39: case 40: case 41: case 42: case 43: case 46: 
              case 47: case 48: case 51: case 52: case 56: case 57: case 58: 
              case 59: case 60: 
                if ((MATCH_w_32_0 >> 5 & 0x1) /* d_26 at 0 */ == 1 && 
                  (0 <= (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ && 
                  (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ < 2)) { 
                  unsigned c3_16 = 
                    (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
                  unsigned neg = addressToPC(MATCH_p);
                  
                  #line 135 "machine/hppa/disassembler.m"
                   {

                              apre += sprintf(apre, "%s", cmpsubdw[c3_16 + dis_c_c_n(neg)]);

                              CONS("c_arith_dw ")

                          }

                  
                  
                  
                } /*opt-block*//*opt-block+*/
                else 
                  goto MATCH_label_g1;  /*opt-block+*/
                
                break;
              case 1: case 2: case 3: case 6: case 18: case 32: case 33: 
              case 35: case 36: case 37: case 44: case 45: case 49: case 50: 
              case 53: case 54: case 55: case 61: case 62: case 63: 
                goto MATCH_label_g0; break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 6 & 0x3f) -- ext6_20 at 0 --*/ 
          break;
        case 10: 
          
          #line 158 "machine/hppa/disassembler.m"
           {

                  }

          
          
          
          break;
        case 32: case 33: case 34: case 35: 
          { 
            unsigned c3_16 = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
            unsigned neg = addressToPC(MATCH_p);
            
            #line 139 "machine/hppa/disassembler.m"
             {

                        apre += sprintf(apre, "%s", cmpsubw[c3_16 + dis_c_c_n(neg)]);

                        CONS("c_cmpb_w ")

                    }

            
            
            
          }
          
          break;
        case 36: 
          goto MATCH_label_g1; break;
        case 37: case 44: case 45: 
          goto MATCH_label_g1; break;
        case 39: case 47: 
          { 
            unsigned c3_16 = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
            unsigned neg = addressToPC(MATCH_p);
            
            #line 143 "machine/hppa/disassembler.m"
             {

                        apre += sprintf(apre, "%s", cmpsubdw[c3_16 + dis_c_c_n(neg)]);

                        CONS("c_cmpb_dw ")

                    }

            
            
            
          }
          
          break;
        case 40: case 41: case 42: case 43: 
          goto MATCH_label_g1; break;
        case 48: case 49: 
          if ((MATCH_w_32_0 >> 13 & 0x1) /* d_18 at 0 */ == 1) { 
            unsigned c_16 = (MATCH_w_32_0 >> 15 & 0x1) /* c_16 at 0 */;
            
            #line 155 "machine/hppa/disassembler.m"
             {

                        apre += sprintf(apre, "[%d]",1-c_16);

                        CONS("c_bbs_dw ")

                    }

            
            
            
          } /*opt-block*//*opt-block+*/
          else { 
            unsigned c_16 = (MATCH_w_32_0 >> 15 & 0x1) /* c_16 at 0 */;
            
            #line 151 "machine/hppa/disassembler.m"
             {

                        apre += sprintf(apre, "[%d]",1-c_16);

                        CONS("c_bbs_w ")

                    }

            
            
            
          } /*opt-block*//*opt-block+*/
          
          break;
        case 50: case 51: 
          goto MATCH_label_g2; break;
        case 52: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 0 && 
            (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 1) 
            goto MATCH_label_g0;  /*opt-block+*/
          else 
            goto MATCH_label_g2;  /*opt-block+*/
          
          break;
        case 53: 
          goto MATCH_label_g2; break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_g; 
  
  MATCH_label_g0: (void)0; /*placeholder for label*/ 
    
    #line 161 "machine/hppa/disassembler.m"
    {

                astr += sprintf(astr, "#c_C%08X", getDword(hostpc));

            }

    
     
    goto MATCH_finished_g; 
    
  MATCH_label_g1: (void)0; /*placeholder for label*/ 
    { 
      unsigned c3_16 = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
      unsigned neg = addressToPC(MATCH_p);
      
      #line 131 "machine/hppa/disassembler.m"
       {

                  apre += sprintf(apre, "%s", cmpsubw[c3_16 + dis_c_c_n(neg)]);

                  CONS("c_arith_w ")

              }

      
      
      
    } 
    goto MATCH_finished_g; 
    
  MATCH_label_g2: (void)0; /*placeholder for label*/ 
    { 
      unsigned c3_16 = 
        ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ << 2) + 
        (MATCH_w_32_0 >> 13 & 0x3) /* c2_17 at 0 */;
      
      #line 147 "machine/hppa/disassembler.m"
       {

                  apre += sprintf(apre, "%s", logw[c3_16]);

                  CONS("c_sep ")

              }

      
      
      
    } 
    goto MATCH_finished_g; 
    
  MATCH_finished_g: (void)0; /*placeholder for label*/
  
}

#line 165 "machine/hppa/disassembler.m"
}

void NJMCDecoder::dis_c_xd(ADDRESS hostpc)
{



#line 169 "machine/hppa/disassembler.m"
{ 
  dword MATCH_p = 
    
    #line 169 "machine/hppa/disassembler.m"
    hostpc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    
      switch((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */) {
        case 0: case 1: case 2: case 4: case 5: case 6: case 7: case 8: 
        case 10: case 12: case 13: case 14: case 15: case 20: case 21: 
        case 22: case 23: case 28: case 29: case 30: case 31: case 32: 
        case 33: case 34: case 35: case 36: case 37: case 38: case 39: 
        case 40: case 41: case 42: case 43: case 44: case 45: case 46: 
        case 47: case 48: case 49: case 50: case 51: case 52: case 53: 
        case 54: case 55: case 58: case 59: case 60: case 61: case 62: 
        case 63: 
          goto MATCH_label_f0; break;
        case 3: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) 
            if (8 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
              (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 16) { 
              unsigned i = 
                (sign_extend((MATCH_w_32_0 & 0x1) /* i_31 at 0 */, 1) << 4) + 
                (MATCH_w_32_0 >> 1 & 0xf) /* im4_27 at 0 */;
              
              #line 199 "machine/hppa/disassembler.m"
               {

                          astr += sprintf(astr, "%d", i);

                          CONS("s_addr_r_im ")

                      }

              
              
              
            } /*opt-block*//*opt-block+*/
            else 
              goto MATCH_label_f4;  /*opt-block+*/ /*opt-block+*/
          else 
            
              switch((MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */) {
                case 0: 
                  if ((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1) { 
                    unsigned x = (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;
                    
                    #line 175 "machine/hppa/disassembler.m"
                     {

                                apre += sprintf(apre, "%s", ",s");

                                astr += sprintf(astr, "%s", x_11_names[x]);

                                CONS("x_addr_s_byte ")

                            }

                    
                    
                    
                  } /*opt-block*//*opt-block+*/
                  else 
                    goto MATCH_label_f1;  /*opt-block+*/
                  
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

                    
                    
                    
                  } /*opt-block*//*opt-block+*/
                  else 
                    goto MATCH_label_f1;  /*opt-block+*/
                  
                  break;
                case 2: case 6: 
                  if ((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1) 
                    goto MATCH_label_f2;  /*opt-block+*/
                  else 
                    goto MATCH_label_f1;  /*opt-block+*/
                  
                  break;
                case 3: case 4: case 5: case 7: 
                  if ((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1) 
                    goto MATCH_label_f3;  /*opt-block+*/
                  else 
                    goto MATCH_label_f1;  /*opt-block+*/
                  
                  break;
                case 8: case 9: case 10: case 11: case 12: case 13: case 14: 
                case 15: 
                  goto MATCH_label_f0; break;
                default: assert(0);
              } /* (MATCH_w_32_0 >> 6 & 0xf) -- ext4_22 at 0 --*/  
          break;
        case 9: 
          if ((MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ == 0) 
            if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) 
              goto MATCH_label_f4;  /*opt-block+*/
            else 
              if ((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1 && 
                (0 <= (MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ && 
                (MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ < 2)) 
                goto MATCH_label_f2;  /*opt-block+*/
              else 
                goto MATCH_label_f1;  /*opt-block+*/ /*opt-block+*/ 
          else 
            goto MATCH_label_f0;  /*opt-block+*/
          break;
        case 11: 
          if ((MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ == 0) 
            if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) 
              goto MATCH_label_f4;  /*opt-block+*/
            else 
              if ((MATCH_w_32_0 >> 13 & 0x1) /* u_18 at 0 */ == 1 && 
                (0 <= (MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ && 
                (MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ < 2)) 
                goto MATCH_label_f3;  /*opt-block+*/
              else 
                goto MATCH_label_f1;  /*opt-block+*/ /*opt-block+*/ 
          else 
            goto MATCH_label_f0;  /*opt-block+*/
          break;
        case 16: case 17: case 18: case 19: case 24: case 25: case 26: 
        case 27: 
          { 
            unsigned i = 
              (sign_extend((MATCH_w_32_0 & 0x1) /* i_31 at 0 */, 1) << 13) + 
              (MATCH_w_32_0 >> 1 & 0x1fff) /* im13_18 at 0 */;
            
            #line 203 "machine/hppa/disassembler.m"
             {

                        astr += sprintf(astr, "%d", i);

                        CONS("l_addr_16_old ")

                    }

            
            
            
          }
          
          break;
        case 56: case 57: 
          { 
            unsigned i = 
              ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 10) + 
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
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_f; 
  
  MATCH_label_f0: (void)0; /*placeholder for label*/ 
    
    #line 211 "machine/hppa/disassembler.m"
    {

                apre += sprintf(apre, "#c_X_ADDR_SHIFT%08X", getDword(hostpc));

                astr += sprintf(astr, "#c_X_ADDR_SHIFT%08X", getDword(hostpc));

            }

    
     
    goto MATCH_finished_f; 
    
  MATCH_label_f1: (void)0; /*placeholder for label*/ 
    { 
      unsigned x = (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;
      
      #line 171 "machine/hppa/disassembler.m"
         {

                  astr += sprintf(astr, "%s", x_11_names[x]);

                  CONS("x_addr_nots ")

              }

      
      
      
    } 
    goto MATCH_finished_f; 
    
  MATCH_label_f2: (void)0; /*placeholder for label*/ 
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
    
  MATCH_label_f3: (void)0; /*placeholder for label*/ 
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
    
  MATCH_label_f4: (void)0; /*placeholder for label*/ 
    { 
      unsigned i = 
        (sign_extend((MATCH_w_32_0 >> 16 & 0x1) /* im1_15 at 0 */, 1) << 4) + 
        (MATCH_w_32_0 >> 17 & 0xf) /* im4_11 at 0 */;
      
      #line 195 "machine/hppa/disassembler.m"
       {

                  astr += sprintf(astr, "%d", i);

                  CONS("s_addr_im_r ")

              }

      
      
      
    } 
    goto MATCH_finished_f; 
    
  MATCH_finished_f: (void)0; /*placeholder for label*/
  
}

#line 216 "machine/hppa/disassembler.m"
}

void NJMCDecoder::dis_c_wcr(ADDRESS hostpc)
{
    unsigned long regl;


#line 220 "machine/hppa/disassembler.m"
{ 
  dword MATCH_p = 
    
    #line 220 "machine/hppa/disassembler.m"
    hostpc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 5 & 0xff) /* ext8_19 at 0 */ == 69) 
      if ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 0) 
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

          
          
          
        } /*opt-block*//*opt-block+*/ /*opt-block+*/
      else 
        goto MATCH_label_e0;  /*opt-block+*/ 
    else 
      goto MATCH_label_e0;  /*opt-block+*/
    
  }goto MATCH_finished_e; 
  
  MATCH_label_e0: (void)0; /*placeholder for label*/ 
    
    #line 231 "machine/hppa/disassembler.m"
    {

            regl = 0;

            apre += sprintf(apre, "#c_WCR%08X#", getDword(hostpc));

        }

    
     
    goto MATCH_finished_e; 
    
  MATCH_finished_e: (void)0; /*placeholder for label*/
  
}

#line 236 "machine/hppa/disassembler.m"
  astr += sprintf(astr, "%s", cr_06_names[regl]);
}

void NJMCDecoder::dis_c_null(ADDRESS hostpc)
{


#line 240 "machine/hppa/disassembler.m"
{ 
  dword MATCH_p = 
    
    #line 240 "machine/hppa/disassembler.m"
    hostpc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    
      switch((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */) {
        case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
        case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: 
        case 16: case 17: case 18: case 19: case 20: case 21: case 22: 
        case 23: case 24: case 25: case 26: case 27: case 28: case 29: 
        case 30: case 31: case 36: case 37: case 38: case 44: case 45: 
        case 46: case 52: case 53: case 54: case 55: case 60: case 61: 
        case 62: case 63: 
          
          #line 248 "machine/hppa/disassembler.m"
          
                      apre += sprintf(apre, "#c_NULL%08X#", getDword(hostpc));

          
          
          
          break;
        case 32: case 33: case 34: case 35: case 39: case 40: case 41: 
        case 42: case 43: case 47: case 48: case 49: case 50: case 51: 
        case 56: case 57: case 58: case 59: 
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
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_d; 
  
  MATCH_finished_d: (void)0; /*placeholder for label*/
  
}

#line 252 "machine/hppa/disassembler.m"
}

void NJMCDecoder::dis_c_bit(ADDRESS hostpc)
{


#line 255 "machine/hppa/disassembler.m"
{ 
  dword MATCH_p = 
    
    #line 255 "machine/hppa/disassembler.m"
    hostpc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 48) 
      goto MATCH_label_c0;  /*opt-block+*/
    else 
      switch((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */) {
        case 50: case 51: case 52: case 53: case 54: case 55: case 56: 
        case 57: case 58: case 59: case 60: case 61: case 62: case 63: 
          goto MATCH_label_c0; break;
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

                        astr += sprintf(astr, "@%d",p_06);

                        CONS("c_bitpos_w ")

                    }

            
            
            
          } /*opt-block*//*opt-block+*/
          else 
            goto MATCH_label_c0;  /*opt-block+*/
          
          break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_c; 
  
  MATCH_label_c0: (void)0; /*placeholder for label*/ 
    
    #line 264 "machine/hppa/disassembler.m"
    
                astr += sprintf(astr, "#c_BIT%08X#", getDword(hostpc));

    
     
    goto MATCH_finished_c; 
    
  MATCH_finished_c: (void)0; /*placeholder for label*/
  
}

#line 268 "machine/hppa/disassembler.m"
}

void NJMCDecoder::dis_c_addr(ADDRESS hostpc)
{


#line 271 "machine/hppa/disassembler.m"
{ 
  dword MATCH_p = 
    
    #line 271 "machine/hppa/disassembler.m"
    hostpc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (28 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64) 
      goto MATCH_label_b0;  /*opt-block+*/
    else 
      switch((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */) {
        case 0: case 1: case 2: case 4: case 5: case 6: case 7: case 8: 
        case 10: case 12: case 13: case 14: case 15: case 20: case 21: 
        case 22: case 23: 
          goto MATCH_label_b0; break;
        case 3: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1) 
              if ((MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1) 
                if (12 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
                  (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 14) 
                  
                  #line 295 "machine/hppa/disassembler.m"
                   {

                              apre += sprintf(apre, ",me");

                              CONS("c_y_addr_me ")

                          }

                  
                   /*opt-block+*/
                else 
                  goto MATCH_label_b5;  /*opt-block+*/ /*opt-block+*/
              else 
                if (12 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
                  (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 14) 
                  
                  #line 299 "machine/hppa/disassembler.m"
                   {

                              apre += sprintf(apre, ",m");

                              CONS("c_y_addr_m ")

                          }

                  
                   /*opt-block+*/
                else 
                  goto MATCH_label_b4;  /*opt-block+*/ /*opt-block+*/ 
            else 
              if (12 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
                (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 14) 
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

                  
                   /*opt-block+*/ /*opt-block+*/
              else 
                goto MATCH_label_b3;  /*opt-block+*/  
          else 
            if (0 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
              (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 8) 
              if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1) 
                goto MATCH_label_b2;  /*opt-block+*/
              else 
                goto MATCH_label_b1;  /*opt-block+*/ /*opt-block+*/
            else 
              goto MATCH_label_b0;  /*opt-block+*/ 
          break;
        case 9: case 11: 
          if ((MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ == 0) 
            if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) 
              if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1) 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1 && 
                  (0 <= (MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ && 
                  (MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ < 2)) 
                  goto MATCH_label_b5;  /*opt-block+*/
                else 
                  goto MATCH_label_b4;  /*opt-block+*/ /*opt-block+*/
              else 
                goto MATCH_label_b3;  /*opt-block+*/ 
            else 
              if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
                (0 <= (MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ && 
                (MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ < 2)) 
                goto MATCH_label_b2;  /*opt-block+*/
              else 
                goto MATCH_label_b1;  /*opt-block+*/ /*opt-block+*/ 
          else 
            goto MATCH_label_b0;  /*opt-block+*/
          break;
        case 16: case 17: case 18: case 19: case 24: case 25: case 26: 
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
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_b; 
  
  MATCH_label_b0: (void)0; /*placeholder for label*/ 
    
    #line 323 "machine/hppa/disassembler.m"
    
                // Do nothing; no completer

                CONS("BUG!!");

    
     
    goto MATCH_finished_b; 
    
  MATCH_label_b1: (void)0; /*placeholder for label*/ 
    
    #line 288 "machine/hppa/disassembler.m"
     {

                CONS("c_x_addr_notm ")

            }

    
     
    goto MATCH_finished_b; 
    
  MATCH_label_b2: (void)0; /*placeholder for label*/ 
    
    #line 284 "machine/hppa/disassembler.m"
     {

                apre += sprintf(apre, ",x");

                CONS("c_x_addr_m ")

            }

    
     
    goto MATCH_finished_b; 
    
  MATCH_label_b3: (void)0; /*placeholder for label*/ 
    
    #line 281 "machine/hppa/disassembler.m"
     {

                CONS("c_s_addr_notm ")

            }

    
     
    goto MATCH_finished_b; 
    
  MATCH_label_b4: (void)0; /*placeholder for label*/ 
    
    #line 277 "machine/hppa/disassembler.m"
     {

                apre += sprintf(apre, ",ma");

                CONS("c_s_addr_ma ")

            }

    
     
    goto MATCH_finished_b; 
    
  MATCH_label_b5: (void)0; /*placeholder for label*/ 
    
    #line 273 "machine/hppa/disassembler.m"
     {

                apre += sprintf(apre, ",mb");

                CONS("c_s_addr_mb ")

            }

    
     
    goto MATCH_finished_b; 
    
  MATCH_finished_b: (void)0; /*placeholder for label*/
  
}

#line 327 "machine/hppa/disassembler.m"
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



#line 372 "machine/hppa/disassembler.m"
{ 
  dword MATCH_p = 
    
    #line 372 "machine/hppa/disassembler.m"
    hostPC
    ;
  char *MATCH_name;
  static char *MATCH_name_ext8_19_0[] = {
    "BREAK", (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, "MFSP", (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "RFI", (char *)0, 
    (char *)0, (char *)0, (char *)0, "RFI.r", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, "SSM", (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, "RSM", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, "LDSID", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "MFIA", (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, "MTSP", "MTCTL", "MTSM", (char *)0, (char *)0, 
    "MTSARCM", 
  };
  static char *MATCH_name_ext5_11_1[] = {
    "SYNC", (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, "SYNCDMA", 
  };
  static char *MATCH_name_ext_17_2[] = {"MFCTL", "MFCTL.w", };
  static char *MATCH_name_ext4_22_55[] = {
    "LDBS", "LDHS", "LDWS", "LDDS", "LDDAS", "LDCDS", "LDWAS", "LDCWS", 
    "STBS", "STHS", "STWS", "STDS", "STBYS", "STDBYS", "STWAS", "STWDS", 
  };
  static char *MATCH_name_op_58[] = {
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, "LDIL", (char *)0, "ADDIL", (char *)0, (char *)0, 
    "LDO", (char *)0, (char *)0, "LDB", "LDH", "LDW", "LDWM", (char *)0, 
    (char *)0, (char *)0, (char *)0, "STB", "STH", "STW", "STWM", 
  };
  static char *MATCH_name_sub_16_71[] = {
    "FID", "FTEST", "FCPY", "FABS", "FSQRT", "FRND", "FNEG", "FNEGABS", 
  };
  static char *MATCH_name_sub_14_72[] = {
    "FCNVFF", "FCNVXF", "FCNVFX", "FCNVFXT", 
  };
  static char *MATCH_name_sub_16_73[] = {"FCMP", "FSUB", "FMPY", "FDIV", };
  static char *MATCH_name_sub_16_74[] = {"FADD", "FTEST.E", };
  static char *MATCH_name_sub_14_75[] = {
    "FCNVFF.E", "FCNVXF.E", "FCNVFX.E", "FCNVFXT.E", 
  };
  static char *MATCH_name_p_20_98[] = {"VSHD", "SHD", };
  static char *MATCH_name_se_21_99[] = {"VEXTRU", "VEXTRS", };
  static char *MATCH_name_se_21_100[] = {"EXTRU", "EXTRS", };
  static char *MATCH_name_se_21_101[] = {"ZVDEP", "VDEP", };
  static char *MATCH_name_se_21_102[] = {"ZDEP", "DEP", };
  static char *MATCH_name_se_21_103[] = {"ZVDEPI", "VDEPI", };
  static char *MATCH_name_se_21_104[] = {"ZDEPI", "DEPI", };
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    
      switch((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */) {
        case 0: 
          if (199 <= (MATCH_w_32_0 >> 5 & 0xff) /* ext8_19 at 0 */ && 
            (MATCH_w_32_0 >> 5 & 0xff) /* ext8_19 at 0 */ < 256) 
            goto MATCH_label_a0;  /*opt-block+*/
          else 
            switch((MATCH_w_32_0 >> 5 & 0xff) /* ext8_19 at 0 */) {
              case 0: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned im13_06 = 
                    (MATCH_w_32_0 >> 13 & 0x1fff) /* im13_06 at 0 */;
                  unsigned im5_27 = (MATCH_w_32_0 & 0x1f) /* im5_27 at 0 */;
                  
                  #line 544 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              astr += sprintf(astr, "%d,%d", im5_27,im13_06);

                              CONS("BREAK ")

                          }

                  
                  
                  
                }
                
                break;
              case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: 
              case 9: case 10: case 11: case 12: case 13: case 14: case 15: 
              case 16: case 17: case 18: case 19: case 20: case 21: case 22: 
              case 23: case 24: case 25: case 26: case 27: case 28: case 29: 
              case 30: case 31: case 33: case 34: case 35: case 36: case 38: 
              case 39: case 40: case 41: case 42: case 43: case 44: case 45: 
              case 46: case 47: case 48: case 49: case 50: case 51: case 52: 
              case 53: case 54: case 55: case 56: case 57: case 58: case 59: 
              case 60: case 61: case 62: case 63: case 64: case 65: case 66: 
              case 67: case 68: case 70: case 71: case 72: case 73: case 74: 
              case 75: case 76: case 77: case 78: case 79: case 80: case 81: 
              case 82: case 83: case 84: case 85: case 86: case 87: case 88: 
              case 89: case 90: case 91: case 92: case 93: case 94: case 95: 
              case 97: case 98: case 99: case 100: case 102: case 103: 
              case 104: case 105: case 106: case 108: case 109: case 110: 
              case 111: case 112: case 113: case 114: case 116: case 117: 
              case 118: case 119: case 120: case 121: case 122: case 123: 
              case 124: case 125: case 126: case 127: case 128: case 129: 
              case 130: case 131: case 132: case 134: case 135: case 136: 
              case 137: case 138: case 139: case 140: case 141: case 142: 
              case 143: case 144: case 145: case 146: case 147: case 148: 
              case 149: case 150: case 151: case 152: case 153: case 154: 
              case 155: case 156: case 157: case 158: case 159: case 160: 
              case 161: case 162: case 163: case 164: case 166: case 167: 
              case 168: case 169: case 170: case 171: case 172: case 173: 
              case 174: case 175: case 176: case 177: case 178: case 179: 
              case 180: case 181: case 182: case 183: case 184: case 185: 
              case 186: case 187: case 188: case 189: case 190: case 191: 
              case 192: case 196: case 197: 
                goto MATCH_label_a0; break;
              case 32: 
                if ((MATCH_w_32_0 >> 16 & 0x1f) /* ext5_11 at 0 */ == 0 || 
                  (MATCH_w_32_0 >> 16 & 0x1f) /* ext5_11 at 0 */ == 10) { 
                  MATCH_name = 
                    MATCH_name_ext5_11_1[(MATCH_w_32_0 >> 16 & 0x1f) 
                        /* ext5_11 at 0 */]; 
                  goto MATCH_label_a1; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 37: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned sr = 
                    ((MATCH_w_32_0 >> 13 & 0x1) /* s_18 at 0 */ << 2) + 
                    (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
                  unsigned t_27 = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                  
                  #line 591 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              astr += sprintf(astr, "%s,%s", s3_16_names[sr],

                                t_27_names[t_27]);

                              CONS("MFSP ")

                          }

                  
                  
                  
                }
                
                break;
              case 69: 
                MATCH_name = MATCH_name_ext_17_2[(MATCH_w_32_0 >> 14 & 0x1) 
                      /* ext_17 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned cmplt = addressToPC(MATCH_p);
                  unsigned t_27 = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                  
                  #line 563 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              dis_c_wcr(cmplt);

                              astr += sprintf(astr, ",%s", t_27_names[t_27]);

                              CONS("sysop_cr_t ")

                          }

                  
                  
                  
                }
                
                break;
              case 96: case 101: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                goto MATCH_label_a1; 
                
                break;
              case 107: case 115: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned im10_06 = 
                    (MATCH_w_32_0 >> 16 & 0x3ff) /* im10_06 at 0 */;
                  unsigned t_27 = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                  
                  #line 549 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              astr += sprintf(astr, "%d,%s", im10_06,t_27_names[t_27]);

                              CONS("sysop_i_t ");

                          }

                  
                  
                  
                }
                
                break;
              case 133: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned b_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
                  unsigned s2_16 = 
                    (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
                  unsigned t_27 = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                  
                  #line 579 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              astr += sprintf(astr, "(%s,%s),%s", s2_16_names[s2_16],

                                b_06_names[b_06], t_27_names[t_27]);

                              CONS("LDSID ")

                          }

                  
                  
                  
                }
                
                break;
              case 165: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned t_27 = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                  
                  #line 574 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              astr += sprintf(astr, "%s", t_27_names[t_27]);

                              CONS("MFIA ")

                          }

                  
                  
                  
                }
                
                break;
              case 193: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                  unsigned sr = 
                    ((MATCH_w_32_0 >> 13 & 0x1) /* s_18 at 0 */ << 2) + 
                    (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
                  
                  #line 585 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              astr += sprintf(astr, "%s,%s", r_11_names[r_11],

                                s3_16_names[sr]);

                              CONS("MTSP ")

                          }

                  
                  
                  
                }
                
                break;
              case 194: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned cr_06 = 
                    (MATCH_w_32_0 >> 21 & 0x1f) /* cr_06 at 0 */;
                  unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                  
                  #line 569 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              astr += sprintf(astr, "%s,%s", r_11_names[r_11],cr_06_names[cr_06]);

                              CONS("MTCTL ")

                          }

                  
                  
                  
                }
                
                break;
              case 195: case 198: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                  
                  #line 558 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              astr += sprintf(astr, "%s", r_11_names[r_11]);

                              CONS("sysop_r ")

                          }

                  
                  
                  
                }
                
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 5 & 0xff) -- ext8_19 at 0 --*/ 
          break;
        case 1: case 4: case 5: case 6: case 7: case 15: case 20: case 21: 
        case 22: case 23: case 28: case 29: case 30: case 31: case 38: 
        case 46: case 50: case 51: case 54: case 55: case 56: case 57: 
        case 60: case 61: case 62: case 63: 
          goto MATCH_label_a0; break;
        case 2: 
          
            switch((MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */) {
              case 0: 
                MATCH_name = "ANDCM"; goto MATCH_label_a2; break;
              case 1: case 2: case 3: case 6: case 18: case 32: case 33: 
              case 35: case 36: case 37: case 44: case 45: case 49: case 50: 
              case 53: case 54: case 55: case 61: case 62: case 63: 
                goto MATCH_label_a0; break;
              case 4: 
                MATCH_name = "HSUB.u"; goto MATCH_label_a2; break;
              case 5: 
                MATCH_name = "HSUB.s"; goto MATCH_label_a2; break;
              case 7: 
                MATCH_name = "HSUB"; goto MATCH_label_a2; break;
              case 8: 
                MATCH_name = "AND"; goto MATCH_label_a2; break;
              case 9: 
                if ((MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */ == 0) 
                  if ((MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */ == 0) 
                    if ((MATCH_w_32_0 & 0x1f) /* t_27 at 0 */ == 0) 
                      
                      #line 374 "machine/hppa/disassembler.m"
                       {

                                  apre += sprintf(apre, "%s", "NOP");

                                  CONS("NOP ")

                              }

                      
                       /*opt-block+*/
                    else { 
                      MATCH_name = "OR"; 
                      goto MATCH_label_a2; 
                      
                    } /*opt-block*/ /*opt-block+*/
                  else { 
                    unsigned r = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                    unsigned t = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                    
                    #line 378 "machine/hppa/disassembler.m"
                     {

                                apre += sprintf(apre, "%s", "COPY");

                                astr += sprintf(astr, "%s,%s", r_06_names[r], t_27_names[t]);

                                CONS("COPY ")

                             }

                    
                    
                    
                  } /*opt-block*//*opt-block+*/ 
                else { MATCH_name = "OR"; goto MATCH_label_a2; } /*opt-block*/
                break;
              case 10: 
                MATCH_name = "XOR"; goto MATCH_label_a2; break;
              case 11: 
                MATCH_name = "HAVG"; goto MATCH_label_a2; break;
              case 12: 
                MATCH_name = "HADD.u"; goto MATCH_label_a2; break;
              case 13: 
                MATCH_name = "HADD.s"; goto MATCH_label_a2; break;
              case 14: 
                MATCH_name = "UXOR"; goto MATCH_label_a2; break;
              case 15: 
                MATCH_name = "HADD"; goto MATCH_label_a2; break;
              case 16: 
                MATCH_name = "SUB"; goto MATCH_label_a2; break;
              case 17: 
                MATCH_name = "DS"; goto MATCH_label_a2; break;
              case 19: 
                MATCH_name = "SUB.t"; goto MATCH_label_a2; break;
              case 20: 
                MATCH_name = "SUB.b"; goto MATCH_label_a2; break;
              case 21: 
                MATCH_name = "HSHR1ADD"; goto MATCH_label_a2; break;
              case 22: 
                MATCH_name = "HSHR2ADD"; goto MATCH_label_a2; break;
              case 23: 
                MATCH_name = "HSHR3ADD"; goto MATCH_label_a2; break;
              case 24: 
                MATCH_name = "ADD"; goto MATCH_label_a2; break;
              case 25: 
                MATCH_name = "SHL1ADD"; goto MATCH_label_a2; break;
              case 26: 
                MATCH_name = "SHL2ADD"; goto MATCH_label_a2; break;
              case 27: 
                MATCH_name = "SHL3ADD"; goto MATCH_label_a2; break;
              case 28: 
                MATCH_name = "ADD.c"; goto MATCH_label_a2; break;
              case 29: 
                MATCH_name = "HSHL1ADD"; goto MATCH_label_a2; break;
              case 30: 
                MATCH_name = "HSHL2ADD"; goto MATCH_label_a2; break;
              case 31: 
                MATCH_name = "HSHL3ADD"; goto MATCH_label_a2; break;
              case 34: 
                MATCH_name = "CMPCLR"; goto MATCH_label_a2; break;
              case 38: 
                MATCH_name = "UADDCM"; goto MATCH_label_a2; break;
              case 39: 
                MATCH_name = "UADDCMT"; goto MATCH_label_a2; break;
              case 40: 
                MATCH_name = "ADD.l"; goto MATCH_label_a2; break;
              case 41: 
                MATCH_name = "SHL1ADD.l"; goto MATCH_label_a2; break;
              case 42: 
                MATCH_name = "SHL2ADD.l"; goto MATCH_label_a2; break;
              case 43: 
                MATCH_name = "SHL3ADD.l"; goto MATCH_label_a2; break;
              case 46: 
                MATCH_name = "DCOR"; goto MATCH_label_a2; break;
              case 47: 
                MATCH_name = "IDCOR"; goto MATCH_label_a2; break;
              case 48: 
                MATCH_name = "SUB.v"; goto MATCH_label_a2; break;
              case 51: 
                MATCH_name = "SUB.t.v"; goto MATCH_label_a2; break;
              case 52: 
                MATCH_name = "SUB.b.v"; goto MATCH_label_a2; break;
              case 56: 
                MATCH_name = "ADD.v"; goto MATCH_label_a2; break;
              case 57: 
                MATCH_name = "SHL1ADD.v"; goto MATCH_label_a2; break;
              case 58: 
                MATCH_name = "SHL2ADD.v"; goto MATCH_label_a2; break;
              case 59: 
                MATCH_name = "SHL3ADD.v"; goto MATCH_label_a2; break;
              case 60: 
                MATCH_name = "ADD.c.v"; goto MATCH_label_a2; break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 6 & 0x3f) -- ext6_20 at 0 --*/ 
          break;
        case 3: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1) 
              if (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ && 
                (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ < 2 && 
                (8 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
                (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 16)) { 
                MATCH_name = MATCH_name_ext4_22_55[(MATCH_w_32_0 >> 6 & 0xf) 
                      /* ext4_22 at 0 */]; 
                goto MATCH_label_a4; 
                
              } /*opt-block*/
              else { 
                MATCH_name = MATCH_name_ext4_22_55[(MATCH_w_32_0 >> 6 & 0xf) 
                      /* ext4_22 at 0 */]; 
                goto MATCH_label_a3; 
                
              } /*opt-block*/ /*opt-block+*/
            else 
              
                switch((MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */) {
                  case 0: case 1: case 2: case 3: case 4: case 5: case 6: 
                  case 7: 
                    MATCH_name = 
                      MATCH_name_ext4_22_55[(MATCH_w_32_0 >> 6 & 0xf) 
                          /* ext4_22 at 0 */]; 
                    goto MATCH_label_a3; 
                    
                    break;
                  case 8: case 9: case 10: case 11: case 14: case 15: 
                    MATCH_name = 
                      MATCH_name_ext4_22_55[(MATCH_w_32_0 >> 6 & 0xf) 
                          /* ext4_22 at 0 */]; 
                    goto MATCH_label_a4; 
                    
                    break;
                  case 12: 
                    MATCH_name = "STBYS"; goto MATCH_label_a4; break;
                  case 13: 
                    MATCH_name = "STDBYS"; goto MATCH_label_a4; break;
                  default: assert(0);
                } /* (MATCH_w_32_0 >> 6 & 0xf) -- ext4_22 at 0 --*/   
          else 
            
              switch((MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */) {
                case 0: 
                  MATCH_name = "LDBX"; goto MATCH_label_a3; break;
                case 1: 
                  MATCH_name = "LDHX"; goto MATCH_label_a3; break;
                case 2: 
                  MATCH_name = "LDWX"; goto MATCH_label_a3; break;
                case 3: 
                  MATCH_name = "LDDX"; goto MATCH_label_a3; break;
                case 4: 
                  MATCH_name = "LDDAX"; goto MATCH_label_a3; break;
                case 5: 
                  MATCH_name = "LDCDX"; goto MATCH_label_a3; break;
                case 6: 
                  MATCH_name = "LDWAX"; goto MATCH_label_a3; break;
                case 7: 
                  MATCH_name = "LDCWX"; goto MATCH_label_a3; break;
                case 8: case 9: case 10: case 11: case 12: case 13: case 14: 
                case 15: 
                  goto MATCH_label_a0; break;
                default: assert(0);
              } /* (MATCH_w_32_0 >> 6 & 0xf) -- ext4_22 at 0 --*/  
          break;
        case 8: 
          MATCH_name = 
            MATCH_name_op_58[(MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */]; 
          { 
            char *name = MATCH_name;
            unsigned imm21 = 
              ((MATCH_w_32_0 >> 12 & 0x3) /* im2_18 at 0 */ << 11) + 
              ((MATCH_w_32_0 >> 16 & 0x1f) /* im5_11 at 0 */ << 13) + 
              ((MATCH_w_32_0 >> 14 & 0x3) /* im2_16 at 0 */ << 18) + 
              ((MATCH_w_32_0 >> 1 & 0x7ff) /* im11_20 at 0 */ << 20) + 
              (((MATCH_w_32_0 & 0x1) /* i_31 at 0 */ & 0x1) << 31);
            unsigned t_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
            
            #line 400 "machine/hppa/disassembler.m"
             {

                        ANAME

                        astr += sprintf(astr, "0x%x,%s", imm21, t_06_names[t_06]);

                        CONS("LDIL ")

                    }

            
            
            
          }
          
          break;
        case 9: 
          if ((MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ == 0) 
            if ((MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ == 1) 
              if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
                (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
                (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
                (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
                (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ && 
                (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ < 2)) { 
                MATCH_name = "FSTWS"; 
                goto MATCH_label_a6; 
                
              } /*opt-block*/
              else { 
                MATCH_name = "FSTWX"; 
                goto MATCH_label_a6; 
                
              } /*opt-block*/ /*opt-block+*/
            else 
              if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
                (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
                (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
                (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
                (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ && 
                (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ < 2)) { 
                MATCH_name = "FLDWS"; 
                goto MATCH_label_a5; 
                
              } /*opt-block*/
              else { 
                MATCH_name = "FLDWX"; 
                goto MATCH_label_a5; 
                
              } /*opt-block*/ /*opt-block+*/ 
          else 
            goto MATCH_label_a0;  /*opt-block+*/
          break;
        case 10: 
          MATCH_name = 
            MATCH_name_op_58[(MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */]; 
          { 
            char *name = MATCH_name;
            unsigned imm21 = 
              ((MATCH_w_32_0 >> 12 & 0x3) /* im2_18 at 0 */ << 11) + 
              ((MATCH_w_32_0 >> 16 & 0x1f) /* im5_11 at 0 */ << 13) + 
              ((MATCH_w_32_0 >> 14 & 0x3) /* im2_16 at 0 */ << 18) + 
              ((MATCH_w_32_0 >> 1 & 0x7ff) /* im11_20 at 0 */ << 20) + 
              (((MATCH_w_32_0 & 0x1) /* i_31 at 0 */ & 0x1) << 31);
            unsigned r_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
            
            #line 395 "machine/hppa/disassembler.m"
             {

                        ANAME

                        astr += sprintf(astr, "%d,%s,%s", imm21, r_06_names[r_06], t_11_names[1]);

                        CONS("ADDIL ")

                    }

            
            
            
          }
          
          break;
        case 11: 
          if ((MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ == 0) 
            if ((MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ == 1) 
              if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
                (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
                (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
                (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
                (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ && 
                (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ < 2)) { 
                MATCH_name = "FSTDS"; 
                goto MATCH_label_a8; 
                
              } /*opt-block*/
              else { 
                MATCH_name = "FSTDX"; 
                goto MATCH_label_a8; 
                
              } /*opt-block*/ /*opt-block+*/
            else 
              if ((MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
                (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
                (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
                (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
                (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ && 
                (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ < 2)) { 
                MATCH_name = "FLDDS"; 
                goto MATCH_label_a7; 
                
              } /*opt-block*/
              else { 
                MATCH_name = "FLDDX"; 
                goto MATCH_label_a7; 
                
              } /*opt-block*/ /*opt-block+*/ 
          else 
            goto MATCH_label_a0;  /*opt-block+*/
          break;
        case 12: 
          
            switch((MATCH_w_32_0 >> 9 & 0x3) /* class_21 at 0 */) {
              case 0: 
                if ((MATCH_w_32_0 >> 13 & 0x7) /* sub_16 at 0 */ == 1) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else { 
                  MATCH_name = 
                    MATCH_name_sub_16_71[(MATCH_w_32_0 >> 13 & 0x7) 
                        /* sub_16 at 0 */]; 
                  { 
                    char *name = MATCH_name;
                    unsigned fmt = 
                      (MATCH_w_32_0 >> 11 & 0x3) /* fmt_19 at 0 */;
                    unsigned r = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                    unsigned t = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                    
                    #line 638 "machine/hppa/disassembler.m"
                     {

                                apre += sprintf(apre, "%s", killDot(name));

                                dis_flt_fmt(fmt);

                                astr += sprintf(astr, "%s, ", dis_freg(r));

                                astr += sprintf(astr, "%s",   dis_freg(t));

                                CONS("flt_c0_all ")

                            }

                    
                    
                    
                  }
                  
                } /*opt-block*/
                
                break;
              case 1: 
                if (0 <= (MATCH_w_32_0 >> 15 & 0x7) /* sub_14 at 0 */ && 
                  (MATCH_w_32_0 >> 15 & 0x7) /* sub_14 at 0 */ < 4) { 
                  MATCH_name = 
                    MATCH_name_sub_14_72[(MATCH_w_32_0 >> 15 & 0x7) 
                        /* sub_14 at 0 */]; 
                  { 
                    char *name = MATCH_name;
                    unsigned df = (MATCH_w_32_0 >> 13 & 0x3) /* df_17 at 0 */;
                    unsigned r = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                    unsigned sf = (MATCH_w_32_0 >> 11 & 0x3) /* sf_19 at 0 */;
                    unsigned t = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                    
                    #line 645 "machine/hppa/disassembler.m"
                     {

                                apre += sprintf(apre, "%s", killDot(name));

                                dis_flt_fmt(sf);

                                dis_flt_fmt(df);

                                astr += sprintf(astr, "%s, ", dis_freg(r));

                                astr += sprintf(astr, "%s",   dis_freg(t));

                                CONS("flt_c1_all ")

                            }

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 2: 
                
                  switch((MATCH_w_32_0 >> 13 & 0x7) /* sub_16 at 0 */) {
                    case 0: 
                      MATCH_name = 
                        MATCH_name_sub_16_73[(MATCH_w_32_0 >> 13 & 0x7) 
                            /* sub_16 at 0 */]; 
                      goto MATCH_label_a9; 
                      
                      break;
                    case 1: 
                      MATCH_name = 
                        MATCH_name_sub_16_71[(MATCH_w_32_0 >> 13 & 0x7) 
                            /* sub_16 at 0 */]; 
                      goto MATCH_label_a9; 
                      
                      break;
                    case 2: case 3: case 4: case 5: case 6: case 7: 
                      goto MATCH_label_a0; break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 13 & 0x7) -- sub_16 at 0 --*/ 
                break;
              case 3: 
                
                  switch((MATCH_w_32_0 >> 13 & 0x7) /* sub_16 at 0 */) {
                    case 0: 
                      MATCH_name = 
                        MATCH_name_sub_16_74[(MATCH_w_32_0 >> 13 & 0x7) 
                            /* sub_16 at 0 */]; 
                      goto MATCH_label_a10; 
                      
                      break;
                    case 1: case 2: case 3: 
                      MATCH_name = 
                        MATCH_name_sub_16_73[(MATCH_w_32_0 >> 13 & 0x7) 
                            /* sub_16 at 0 */]; 
                      goto MATCH_label_a10; 
                      
                      break;
                    case 4: case 5: case 6: case 7: 
                      goto MATCH_label_a0; break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 13 & 0x7) -- sub_16 at 0 --*/ 
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 9 & 0x3) -- class_21 at 0 --*/ 
          break;
        case 13: 
          MATCH_name = 
            MATCH_name_op_58[(MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */]; 
          { 
            char *name = MATCH_name;
            unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
            unsigned ldisp = 
              (sign_extend((MATCH_w_32_0 & 0x1) /* i_31 at 0 */, 1) << 13) + 
              (MATCH_w_32_0 >> 1 & 0x1fff) /* im13_18 at 0 */;
            unsigned t = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
            
            #line 440 "machine/hppa/disassembler.m"
             {

                        ANAME

                        astr += sprintf(astr, "%d(%s),%s", ldisp, b_06_names[b], t_06_names[t]);

                        CONS("LDO ")

                    }

            
            
            
          }
          
          break;
        case 14: 
          
            switch((MATCH_w_32_0 >> 9 & 0x3) /* class_21 at 0 */) {
              case 0: 
                goto MATCH_label_a0; break;
              case 1: 
                if (0 <= (MATCH_w_32_0 >> 15 & 0x7) /* sub_14 at 0 */ && 
                  (MATCH_w_32_0 >> 15 & 0x7) /* sub_14 at 0 */ < 4) { 
                  MATCH_name = 
                    MATCH_name_sub_14_75[(MATCH_w_32_0 >> 15 & 0x7) 
                        /* sub_14 at 0 */]; 
                  { 
                    char *name = MATCH_name;
                    unsigned df = (MATCH_w_32_0 >> 13 & 0x3) /* df_17 at 0 */;
                    unsigned r = 
                      ((MATCH_w_32_0 >> 7 & 0x1) /* r1_24 at 0 */ << 5) + 
                      (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                    unsigned sf = (MATCH_w_32_0 >> 11 & 0x3) /* sf_19 at 0 */;
                    unsigned t = 
                      ((MATCH_w_32_0 >> 6 & 0x1) /* t_25 at 0 */ << 5) + 
                      (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                    
                    #line 645 "machine/hppa/disassembler.m"
                     {

                                apre += sprintf(apre, "%s", killDot(name));

                                dis_flt_fmt(sf);

                                dis_flt_fmt(df);

                                astr += sprintf(astr, "%s, ", dis_freg(r));

                                astr += sprintf(astr, "%s",   dis_freg(t));

                                CONS("flt_c1_all ")

                            }

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 2: 
                
                  switch((MATCH_w_32_0 >> 13 & 0x7) /* sub_16 at 0 */) {
                    case 0: 
                      MATCH_name = "FCMP.E"; goto MATCH_label_a11; break;
                    case 1: 
                      MATCH_name = 
                        MATCH_name_sub_16_74[(MATCH_w_32_0 >> 13 & 0x7) 
                            /* sub_16 at 0 */]; 
                      goto MATCH_label_a11; 
                      
                      break;
                    case 2: case 3: case 4: case 5: case 6: case 7: 
                      goto MATCH_label_a0; break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 13 & 0x7) -- sub_16 at 0 --*/ 
                break;
              case 3: 
                
                  switch((MATCH_w_32_0 >> 13 & 0x7) /* sub_16 at 0 */) {
                    case 0: 
                      if ((MATCH_w_32_0 >> 8 & 0x1) /* x_23 at 0 */ == 0) { 
                        MATCH_name = "FADD.E"; 
                        goto MATCH_label_a12; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 1: 
                      if ((MATCH_w_32_0 >> 8 & 0x1) /* x_23 at 0 */ == 0) { 
                        MATCH_name = "FSUB.E"; 
                        goto MATCH_label_a12; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 2: 
                      if ((MATCH_w_32_0 >> 8 & 0x1) /* x_23 at 0 */ == 1) { 
                        unsigned r1 = 
                          ((MATCH_w_32_0 >> 7 & 0x1) /* r1_24 at 0 */ << 5) + 
                          (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                        unsigned r2 = 
                          ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ << 5) + 
                          (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                        unsigned t = 
                          ((MATCH_w_32_0 >> 6 & 0x1) /* t_25 at 0 */ << 5) + 
                          (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                        
                        #line 671 "machine/hppa/disassembler.m"
                         {

                                    apre += sprintf(apre, "XMPYU");

                                    astr += sprintf(astr, "%s, ", dis_freg(r1));

                                    astr += sprintf(astr, "%s, ", dis_freg(r2));

                                    astr += sprintf(astr, "%s",   dis_freg(t));

                                    CONS("FMPYU ");

                                }

                        

                        // Floating point loads and stores

                        
                        
                        
                      } /*opt-block*//*opt-block+*/
                      else { 
                        MATCH_name = "FMPY.E"; 
                        goto MATCH_label_a12; 
                        
                      } /*opt-block*/
                      
                      break;
                    case 3: 
                      if ((MATCH_w_32_0 >> 8 & 0x1) /* x_23 at 0 */ == 0) { 
                        MATCH_name = "FDIV.E"; 
                        goto MATCH_label_a12; 
                        
                      } /*opt-block*/
                      else 
                        goto MATCH_label_a0;  /*opt-block+*/
                      
                      break;
                    case 4: case 5: case 6: case 7: 
                      goto MATCH_label_a0; break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 13 & 0x7) -- sub_16 at 0 --*/ 
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 9 & 0x3) -- class_21 at 0 --*/ 
          break;
        case 16: case 17: case 18: case 19: 
          MATCH_name = 
            MATCH_name_op_58[(MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */]; 
          { 
            char *name = MATCH_name;
            unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
            unsigned c_addr = addressToPC(MATCH_p);
            unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
            unsigned t_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
            unsigned xd = addressToPC(MATCH_p);
            
            #line 422 "machine/hppa/disassembler.m"
             {

                        ANAME

                        IGNORE(c_addr)

                        dis_c_xd(xd);

                        astr += sprintf(astr, "(%s,%s),%s", 

                            s2_16_names[s], b_06_names[b],

                            t_11_names[t_11]);

                        CONS("iloads_ldisp ")

                    }

            
            
            
          }
          
          break;
        case 24: case 25: case 26: case 27: 
          MATCH_name = 
            MATCH_name_op_58[(MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */]; 
          { 
            char *name = MATCH_name;
            unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
            unsigned c_addr = addressToPC(MATCH_p);
            unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
            unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
            unsigned xd = addressToPC(MATCH_p);
            
            #line 431 "machine/hppa/disassembler.m"
             {

                        ANAME

                        astr += sprintf(astr, "%s,", r_11_names[r_11]);

                        IGNORE(c_addr)

                        dis_c_xd(xd);

                        astr += sprintf(astr, "(%s,%s)",

                            s2_16_names[s], b_06_names[b]);

                        CONS("istores_ldisp ")

                    }

            
            
            
          }
          
          break;
        case 32: 
          MATCH_name = "CMPBT"; goto MATCH_label_a13; break;
        case 33: 
          MATCH_name = "CMPIBT"; goto MATCH_label_a14; break;
        case 34: 
          MATCH_name = "CMPBF"; goto MATCH_label_a13; break;
        case 35: 
          MATCH_name = "CMPIBF"; goto MATCH_label_a14; break;
        case 36: 
          MATCH_name = "CMPICLR"; goto MATCH_label_a15; break;
        case 37: 
          if ((MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ == 1 && 
            (0 <= (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ && 
            (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ < 2)) { 
            MATCH_name = "SUBI.v"; 
            goto MATCH_label_a15; 
            
          } /*opt-block*/
          else { MATCH_name = "SUBI"; goto MATCH_label_a15; } /*opt-block*/
          
          break;
        case 39: 
          MATCH_name = "CMPBdwt"; goto MATCH_label_a13; break;
        case 40: 
          MATCH_name = "ADDBT"; goto MATCH_label_a16; break;
        case 41: 
          MATCH_name = "ADDIBT"; goto MATCH_label_a17; break;
        case 42: 
          MATCH_name = "ADDBF"; goto MATCH_label_a16; break;
        case 43: 
          MATCH_name = "ADDIBF"; goto MATCH_label_a17; break;
        case 44: 
          if ((MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ == 1 && 
            (0 <= (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ && 
            (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ < 2)) { 
            MATCH_name = "ADDI.t.v"; 
            goto MATCH_label_a15; 
            
          } /*opt-block*/
          else { MATCH_name = "ADDI.t"; goto MATCH_label_a15; } /*opt-block*/
          
          break;
        case 45: 
          if ((MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ == 1 && 
            (0 <= (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ && 
            (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ < 2)) { 
            MATCH_name = "ADDI.v"; 
            goto MATCH_label_a15; 
            
          } /*opt-block*/
          else { MATCH_name = "ADDI"; goto MATCH_label_a15; } /*opt-block*/
          
          break;
        case 47: 
          MATCH_name = "CMPBdwf"; goto MATCH_label_a13; break;
        case 48: 
          goto MATCH_label_a18; break;
        case 49: 
          if ((MATCH_w_32_0 >> 13 & 0x1) /* d_18 at 0 */ == 1) 
            goto MATCH_label_a0;  /*opt-block+*/
          else 
            goto MATCH_label_a18;  /*opt-block+*/
          
          break;
        case 52: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ == 1 && 
              (0 <= (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ && 
              (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ < 2)) { 
              MATCH_name = MATCH_name_se_21_100[(MATCH_w_32_0 >> 10 & 0x1) 
                    /* se_21 at 0 */]; 
              { 
                char *name = MATCH_name;
                unsigned c = addressToPC(MATCH_p);
                unsigned len = 32 - (MATCH_w_32_0 & 0x1f) /* clen5_27 at 0 */;
                unsigned p = (MATCH_w_32_0 >> 5 & 0x1f) /* pos5_22 at 0 */;
                unsigned r = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                unsigned t = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
                
                #line 466 "machine/hppa/disassembler.m"
                 {

                            ANAME

                            dis_c_c(c);

                            astr += sprintf(astr, "%s,%d,%d,%s", r_11_names[r], p, len,

                                t_27_names[t]);

                            CONS("ext_fix ")

                        }

                
                
                
              }
              
            } /*opt-block*/
            else { 
              MATCH_name = MATCH_name_se_21_99[(MATCH_w_32_0 >> 10 & 0x1) 
                    /* se_21 at 0 */]; 
              { 
                char *name = MATCH_name;
                unsigned c = addressToPC(MATCH_p);
                unsigned len = 32 - (MATCH_w_32_0 & 0x1f) /* clen5_27 at 0 */;
                unsigned r = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                unsigned t = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
                
                #line 459 "machine/hppa/disassembler.m"
                 {

                            ANAME

                            dis_c_c(c);

                            astr += sprintf(astr, "%s,%d,%s", r_11_names[r], len,

                                t_27_names[t]);

                            CONS("ext_var ")

                        }

                
                
                
              }
              
            } /*opt-block*/ /*opt-block+*/
          else 
            if ((MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 1) 
              goto MATCH_label_a0;  /*opt-block+*/
            else 
              if ((MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ == 1) { 
                MATCH_name = MATCH_name_p_20_98[(MATCH_w_32_0 >> 11 & 0x1) 
                      /* p_20 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned c = addressToPC(MATCH_p);
                  unsigned p = 
                    31 - (MATCH_w_32_0 >> 5 & 0x1f) /* pos5_22 at 0 */;
                  unsigned r1 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                  unsigned r2 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                  unsigned t = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                  
                  #line 452 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              dis_c_c(c);

                              astr += sprintf(astr, "%s,%s,%d,%s", r_11_names[r1], r_06_names[r2],

                                  p, t_27_names[t]);

                              CONS("SHD ")

                          }

                  
                  
                  
                }
                
              } /*opt-block*/
              else { 
                MATCH_name = MATCH_name_p_20_98[(MATCH_w_32_0 >> 11 & 0x1) 
                      /* p_20 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned c = addressToPC(MATCH_p);
                  unsigned r1 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                  unsigned r2 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                  unsigned t = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                  
                  #line 445 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              dis_c_c(c);

                              astr += sprintf(astr, "%s,%s,%s", r_11_names[r1], r_06_names[r2],

                                  t_27_names[t]);

                              CONS("VSHD ")

                          }

                  
                  
                  
                }
                
              } /*opt-block*/ /*opt-block+*/ 
          break;
        case 53: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ == 1 && 
              (0 <= (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ && 
              (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ < 2)) { 
              MATCH_name = MATCH_name_se_21_104[(MATCH_w_32_0 >> 10 & 0x1) 
                    /* se_21 at 0 */]; 
              { 
                char *name = MATCH_name;
                unsigned c = addressToPC(MATCH_p);
                unsigned i = 
                  ((sign_extend((MATCH_w_32_0 >> 16 & 0x1) /* im1_15 at 0 */, 
                              1) & 0xfffffff) << 4) + 
                  (MATCH_w_32_0 >> 17 & 0xf) /* im4_11 at 0 */;
                unsigned len = 32 - (MATCH_w_32_0 & 0x1f) /* clen5_27 at 0 */;
                unsigned p = 
                  31 - (MATCH_w_32_0 >> 5 & 0x1f) /* pos5_22 at 0 */;
                unsigned t = (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
                
                #line 494 "machine/hppa/disassembler.m"
                 {

                            ANAME

                            dis_c_c(c);

                            astr += sprintf(astr, "%d,%d,%d,%s", i, p, len,

                                t_27_names[t]);

                            CONS("dep_ifix ")

                        }

                
                
                
              }
              
            } /*opt-block*/
            else { 
              MATCH_name = MATCH_name_se_21_103[(MATCH_w_32_0 >> 10 & 0x1) 
                    /* se_21 at 0 */]; 
              { 
                char *name = MATCH_name;
                unsigned c = addressToPC(MATCH_p);
                unsigned i = 
                  ((sign_extend((MATCH_w_32_0 >> 16 & 0x1) /* im1_15 at 0 */, 
                              1) & 0xfffffff) << 4) + 
                  (MATCH_w_32_0 >> 17 & 0xf) /* im4_11 at 0 */;
                unsigned len = 32 - (MATCH_w_32_0 & 0x1f) /* clen5_27 at 0 */;
                unsigned t = (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
                
                #line 487 "machine/hppa/disassembler.m"
                 {

                            ANAME

                            dis_c_c(c);

                            astr += sprintf(astr, "%d,%d,%s", i, len,

                                t_27_names[t]);

                            CONS("dep_ivar ")

                        }

                
                
                
              }
              
            } /*opt-block*/ /*opt-block+*/
          else 
            if ((MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ == 1 && 
              (0 <= (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ && 
              (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ < 2)) { 
              MATCH_name = MATCH_name_se_21_102[(MATCH_w_32_0 >> 10 & 0x1) 
                    /* se_21 at 0 */]; 
              { 
                char *name = MATCH_name;
                unsigned c = addressToPC(MATCH_p);
                unsigned len = 32 - (MATCH_w_32_0 & 0x1f) /* clen5_27 at 0 */;
                unsigned p = 
                  31 - (MATCH_w_32_0 >> 5 & 0x1f) /* pos5_22 at 0 */;
                unsigned r = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                unsigned t = (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
                
                #line 480 "machine/hppa/disassembler.m"
                 {

                            ANAME

                            dis_c_c(c);

                            astr += sprintf(astr, "%s,%d,%d,%s", r_11_names[r], p, len,

                                t_27_names[t]);

                            CONS("dep_fix ")

                        }

                
                
                
              }
              
            } /*opt-block*/
            else { 
              MATCH_name = MATCH_name_se_21_101[(MATCH_w_32_0 >> 10 & 0x1) 
                    /* se_21 at 0 */]; 
              { 
                char *name = MATCH_name;
                unsigned c = addressToPC(MATCH_p);
                unsigned len = 32 - (MATCH_w_32_0 & 0x1f) /* clen5_27 at 0 */;
                unsigned r = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                unsigned t = (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
                
                #line 473 "machine/hppa/disassembler.m"
                 {

                            ANAME

                            dis_c_c(c);

                            astr += sprintf(astr, "%s,%d,%s", r_11_names[r], len,

                                t_27_names[t]);

                            CONS("dep_var ")

                        }

                
                
                
              }
              
            } /*opt-block*/ /*opt-block+*/
          break;
        case 58: 
          
            switch((MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */) {
              case 0: 
                MATCH_name = "BL"; goto MATCH_label_a19; break;
              case 1: 
                MATCH_name = "GATE"; goto MATCH_label_a19; break;
              case 2: 
                if ((MATCH_w_32_0 >> 12 & 0x1) /* ve_19 at 0 */ == 1) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else { 
                  MATCH_name = "BLR"; 
                  { 
                    char *name = MATCH_name;
                    unsigned nulli = addressToPC(MATCH_p);
                    unsigned t_06 = 
                      (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
                    unsigned x_11 = 
                      (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;
                    
                    #line 518 "machine/hppa/disassembler.m"
                     {

                                ANAME

                                dis_c_null(nulli);

                                astr += sprintf(astr, "%s,%s", x_11_names[x_11], t_06_names[t_06]);

                                CONS("BLR ")

                            }

                    
                    
                    
                  }
                  
                } /*opt-block*/
                
                break;
              case 3: 
                goto MATCH_label_a0; break;
              case 4: 
                MATCH_name = "BL.PUSH"; goto MATCH_label_a19; break;
              case 5: 
                MATCH_name = "BL.LONG"; 
                { 
                  char *name = MATCH_name;
                  unsigned nulli = addressToPC(MATCH_p);
                  unsigned ubr_target = 
                    8 + 
                    ((MATCH_w_32_0 >> 3 & 0x3ff) /* w10_19 at 0 */ << 2) + 
                    ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 12) + 
                    ((MATCH_w_32_0 >> 16 & 0x1f) /* w5_11 at 0 */ << 13) + 
                    ((MATCH_w_32_0 >> 21 & 0x1f) /* w5_06 at 0 */ << 18) + 
                    (sign_extend((MATCH_w_32_0 & 0x1) /* w_31 at 0 */, 
                                1) << 23);
                  
                  #line 512 "machine/hppa/disassembler.m"
                   {

                              ANAME

                              dis_c_null(nulli);

                              astr += sprintf(astr, "%d %s", ubr_target, t_06_names[2]);

                              CONS("BL.LONG ")

                          }

                  
                  
                  
                }
                
                break;
              case 6: 
                if ((MATCH_w_32_0 >> 12 & 0x1) /* ve_19 at 0 */ == 1 && 
                  (0 <= (MATCH_w_32_0 >> 1 & 0x1) /* n_30 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x1) /* n_30 at 0 */ < 2)) { 
                  MATCH_name = "BVE"; 
                  goto MATCH_label_a20; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "BV"; 
                  { 
                    char *name = MATCH_name;
                    unsigned b_06 = 
                      (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
                    unsigned nulli = addressToPC(MATCH_p);
                    unsigned x_11 = 
                      (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;
                    
                    #line 524 "machine/hppa/disassembler.m"
                     {

                                ANAME

                                dis_c_null(nulli);

                                astr += sprintf(astr, "%s(%s)", x_11_names[x_11], b_06_names[b_06]);

                                CONS("BV ")

                            }

                    
                    
                    
                  }
                  
                } /*opt-block*/
                
                break;
              case 7: 
                if ((MATCH_w_32_0 >> 12 & 0x1) /* ve_19 at 0 */ == 0) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else { 
                  MATCH_name = "BVE.l"; 
                  goto MATCH_label_a20; 
                  
                } /*opt-block*/
                
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 13 & 0x7) -- ext3_16 at 0 --*/ 
          break;
        case 59: 
          MATCH_name = "cmpib_dw"; goto MATCH_label_a14; break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    
    #line 721 "machine/hppa/disassembler.m"
    
                apre += sprintf(apre, "unrecog. %02X %08X",

                  (getDword(hostPC) >> 26) & 0x3F, getDword(hostPC) & 0x03FFFFFF);

    
     
    goto MATCH_finished_a; 
    
  MATCH_label_a1: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      
      #line 554 "machine/hppa/disassembler.m"
       {

                  ANAME

                  CONS("sysop_simple ");

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a2: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned cmplt = addressToPC(MATCH_p);
      unsigned r_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned t_27 = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      
      #line 383 "machine/hppa/disassembler.m"
       {

                  ANAME

                  dis_c_c(cmplt);

                  astr += sprintf(astr, "%s,%s,%s", r_11_names[r_11],r_06_names[r_06],t_27_names[t_27]);

                  CONS("arith ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a3: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned c_addr = addressToPC(MATCH_p);
      unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned t_27 = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      unsigned xd = addressToPC(MATCH_p);
      
      #line 405 "machine/hppa/disassembler.m"
       {

                  ANAME

                  dis_c_addr(c_addr);         // Can have ,ma/,mb

                  dis_c_xd(xd);

                  astr += sprintf(astr, "(%s,%s),%s",

                      s2_16_names[s], b_06_names[b],

                      t_27_names[t_27]);

                  CONS("iloads ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a4: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned c_addr = addressToPC(MATCH_p);
      unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned xd = addressToPC(MATCH_p);
      
      #line 414 "machine/hppa/disassembler.m"
       {

                  ANAME

                  dis_c_addr(c_addr);         // Can have ,ma/,mb

                  astr += sprintf(astr, "%s,", r_11_names[r_11]);

                  dis_c_xd(xd);

                  astr += sprintf(astr, "(%s,%s)", s2_16_names[s], b_06_names[b]);

                  CONS("istores ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a5: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned c_addr = addressToPC(MATCH_p);
      unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned t_27 = 
        ((MATCH_w_32_0 >> 6 & 0x1) /* t_25 at 0 */ << 5) + 
        (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      unsigned xd = addressToPC(MATCH_p);
      
      #line 680 "machine/hppa/disassembler.m"
       {

                  apre += sprintf(apre, "%s",killDot(name));

      //          dis_c_addr(c_faddr);

                  dis_c_addr(c_addr);

                  dis_c_xd(xd);

                  astr += sprintf(astr, "(%s,%s),%s",

                      s2_16_names[s], b_06_names[b],

                      dis_freg(t_27));

                  CONS("fwloads ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a6: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned c_addr = addressToPC(MATCH_p);
      unsigned r = 
        ((MATCH_w_32_0 >> 6 & 0x1) /* r_25 at 0 */ << 5) + 
        (MATCH_w_32_0 & 0x1f) /* r_27 at 0 */;
      unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned xd = addressToPC(MATCH_p);
      
      #line 690 "machine/hppa/disassembler.m"
       {

                  apre += sprintf(apre, "%s",killDot(name));

      //          dis_c_addr(c_faddr);

                  dis_c_addr(c_addr);

                  astr += sprintf(astr, "%s,", dis_freg(r));

                  dis_c_xd(xd);

                  astr += sprintf(astr, "(%s,%s)",

                      s2_16_names[s], b_06_names[b]);

                  CONS("fwstores ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a7: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned c_addr = addressToPC(MATCH_p);
      unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned t_27 = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      unsigned xd = addressToPC(MATCH_p);
      
      #line 700 "machine/hppa/disassembler.m"
       {

                  apre += sprintf(apre, "%s",killDot(name));

      //          dis_c_addr(c_faddr);

                  dis_c_addr(c_addr);

                  dis_c_xd(xd);

                  astr += sprintf(astr, "(%s,%s),%s",

                      s2_16_names[s], b_06_names[b],

                      dis_freg(t_27));

                  CONS("fdloads ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a8: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned c_addr = addressToPC(MATCH_p);
      unsigned r = (MATCH_w_32_0 & 0x1f) /* r_27 at 0 */;
      unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned xd = addressToPC(MATCH_p);
      
      #line 710 "machine/hppa/disassembler.m"
       {

                  apre += sprintf(apre, "%s", killDot(name));

      //          dis_c_addr(c_faddr);

                  dis_c_addr(c_addr);

                  astr += sprintf(astr, "%s,",   dis_freg(r));

                  dis_c_xd(xd);

                  astr += sprintf(astr, "(%s,%s)",

                      s2_16_names[s], b_06_names[b]);

                  CONS("fdstores ")

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a9: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned c = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      unsigned fmt = (MATCH_w_32_0 >> 11 & 0x3) /* fmt_19 at 0 */;
      unsigned r1 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned r2 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      
      #line 653 "machine/hppa/disassembler.m"
       {

                  apre += sprintf(apre, "%s", killDot(name));

                  dis_flt_fmt(fmt);

                  astr += sprintf(astr, "%s, ", dis_freg(r1));

                  astr += sprintf(astr, "%s",   dis_freg(r2));

      // HACK: Needs completer c decoded

      astr += sprintf(astr, "\t/* Completer c needs decoding */");

      IGNORE(c)

                  CONS("flt_c2_all ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a10: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned fmt = (MATCH_w_32_0 >> 11 & 0x3) /* fmt_19 at 0 */;
      unsigned r1 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned r2 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned t = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      
      #line 663 "machine/hppa/disassembler.m"
       {

                  apre += sprintf(apre, "%s", killDot(name));

                  dis_flt_fmt(fmt);

                  astr += sprintf(astr, "%s, ", dis_freg(r1));

                  astr += sprintf(astr, "%s, ", dis_freg(r2));

                  astr += sprintf(astr, "%s",   dis_freg(t));

                  CONS("flt_c3_all ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a11: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned c = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      unsigned fmt = (MATCH_w_32_0 >> 11 & 0x1) /* f_20 at 0 */;
      unsigned r1 = 
        ((MATCH_w_32_0 >> 7 & 0x1) /* r1_24 at 0 */ << 5) + 
        (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned r2 = 
        ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ << 5) + 
        (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      
      #line 653 "machine/hppa/disassembler.m"
       {

                  apre += sprintf(apre, "%s", killDot(name));

                  dis_flt_fmt(fmt);

                  astr += sprintf(astr, "%s, ", dis_freg(r1));

                  astr += sprintf(astr, "%s",   dis_freg(r2));

      // HACK: Needs completer c decoded

      astr += sprintf(astr, "\t/* Completer c needs decoding */");

      IGNORE(c)

                  CONS("flt_c2_all ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a12: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned fmt = (MATCH_w_32_0 >> 11 & 0x1) /* f_20 at 0 */;
      unsigned r1 = 
        ((MATCH_w_32_0 >> 7 & 0x1) /* r1_24 at 0 */ << 5) + 
        (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned r2 = 
        ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ << 5) + 
        (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned t = 
        ((MATCH_w_32_0 >> 6 & 0x1) /* t_25 at 0 */ << 5) + 
        (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      
      #line 663 "machine/hppa/disassembler.m"
       {

                  apre += sprintf(apre, "%s", killDot(name));

                  dis_flt_fmt(fmt);

                  astr += sprintf(astr, "%s, ", dis_freg(r1));

                  astr += sprintf(astr, "%s, ", dis_freg(r2));

                  astr += sprintf(astr, "%s",   dis_freg(t));

                  CONS("flt_c3_all ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a13: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned c_cmplt = addressToPC(MATCH_p);
      unsigned null_cmplt = addressToPC(MATCH_p);
      unsigned r_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned target = 
        ((MATCH_w_32_0 >> 3 & 0x3ff) /* w10_19 at 0 */ << 2) + 
        ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 12) + 
        ((sign_extend((MATCH_w_32_0 & 0x1) /* w_31 at 0 */, 
                    1) & 0x7ffff) << 13);
      
      #line 613 "machine/hppa/disassembler.m"
       {

                  ANAME

                  dis_c_c(c_cmplt);

                  dis_c_null(null_cmplt);

                  astr += sprintf(astr, "%s,%s,0x%x", r_11_names[r_11],

                    r_06_names[r_06], target + pc + 8);

                  CONS("cmpb_all ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a14: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned c_cmplt = addressToPC(MATCH_p);
      unsigned im5_11 = 
        ((sign_extend((MATCH_w_32_0 >> 16 & 0x1) /* im1_15 at 0 */, 
                    1) & 0xfffffff) << 4) + 
        (MATCH_w_32_0 >> 17 & 0xf) /* im4_11 at 0 */;
      unsigned null_cmplt = addressToPC(MATCH_p);
      unsigned r_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned target = 
        ((MATCH_w_32_0 >> 3 & 0x3ff) /* w10_19 at 0 */ << 2) + 
        ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 12) + 
        ((sign_extend((MATCH_w_32_0 & 0x1) /* w_31 at 0 */, 
                    1) & 0x7ffff) << 13);
      
      #line 621 "machine/hppa/disassembler.m"
       {

                  ANAME

                  dis_c_c(c_cmplt);

                  dis_c_null(null_cmplt);

                  astr += sprintf(astr, "%d,%s,0x%x", im5_11, r_06_names[r_06],

                    target + pc + 8);

                  CONS("cmpib_all ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a15: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned cmplt = addressToPC(MATCH_p);
      unsigned im11_21 = 
        (sign_extend((MATCH_w_32_0 & 0x1) /* i_31 at 0 */, 1) << 10) + 
        (MATCH_w_32_0 >> 1 & 0x3ff) /* im10_21 at 0 */;
      unsigned r_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned t_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
      
      #line 389 "machine/hppa/disassembler.m"
       {

                  ANAME

                  dis_c_c(cmplt);

                  astr += sprintf(astr, "%d,%s,%s", im11_21, r_06_names[r_06], t_11_names[t_11]);

                  CONS("arith_imm ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a16: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned c_cmplt = addressToPC(MATCH_p);
      unsigned null_cmplt = addressToPC(MATCH_p);
      unsigned r_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned target = 
        ((MATCH_w_32_0 >> 3 & 0x3ff) /* w10_19 at 0 */ << 2) + 
        ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 12) + 
        ((sign_extend((MATCH_w_32_0 & 0x1) /* w_31 at 0 */, 
                    1) & 0x7ffff) << 13);
      
      #line 597 "machine/hppa/disassembler.m"
       {

                  ANAME

                  dis_c_c(c_cmplt);

                  dis_c_null(null_cmplt);

                  astr += sprintf(astr, "%s,%s,0x%x", r_11_names[r_11],

                    r_06_names[r_06], target + pc + 8);

                  CONS("addb_all ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a17: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned c_cmplt = addressToPC(MATCH_p);
      unsigned im5 = 
        ((sign_extend((MATCH_w_32_0 >> 16 & 0x1) /* im1_15 at 0 */, 
                    1) & 0xfffffff) << 4) + 
        (MATCH_w_32_0 >> 17 & 0xf) /* im4_11 at 0 */;
      unsigned null_cmplt = addressToPC(MATCH_p);
      unsigned r_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned target = 
        ((MATCH_w_32_0 >> 3 & 0x3ff) /* w10_19 at 0 */ << 2) + 
        ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 12) + 
        ((sign_extend((MATCH_w_32_0 & 0x1) /* w_31 at 0 */, 
                    1) & 0x7ffff) << 13);
      
      #line 605 "machine/hppa/disassembler.m"
       {

                  ANAME

                  dis_c_c(c_cmplt);

                  dis_c_null(null_cmplt);

                  astr += sprintf(astr, "%d,%s,0x%x", im5,

                    r_06_names[r_06], target + pc + 8);

                  CONS("addb_all ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a18: (void)0; /*placeholder for label*/ 
    { 
      unsigned bit_cmplt = addressToPC(MATCH_p);
      unsigned c_cmplt = addressToPC(MATCH_p);
      unsigned null_cmplt = addressToPC(MATCH_p);
      unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned target = 
        ((MATCH_w_32_0 >> 3 & 0x3ff) /* w10_19 at 0 */ << 2) + 
        ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 12) + 
        ((sign_extend((MATCH_w_32_0 & 0x1) /* w_31 at 0 */, 
                    1) & 0x7ffff) << 13);
      
      #line 629 "machine/hppa/disassembler.m"
       {

                  apre += sprintf(apre, "%s", "BB");

                  dis_c_c(c_cmplt);

                  dis_c_null(null_cmplt);

                  astr += sprintf(astr, "%s,", r_11_names[r_11]);

                  dis_c_bit(bit_cmplt);

                  sprintf(astr, ",%x", target + pc + 8);

                  CONS("bb_all ")

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a19: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned nulli = addressToPC(MATCH_p);
      unsigned t_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
      unsigned ubr_target = 
        8 + ((MATCH_w_32_0 >> 3 & 0x3ff) /* w10_19 at 0 */ << 2) + 
        ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 12) + 
        ((MATCH_w_32_0 >> 16 & 0x1f) /* w5_11 at 0 */ << 13) + 
        (sign_extend((MATCH_w_32_0 & 0x1) /* w_31 at 0 */, 1) << 18);
      
      #line 501 "machine/hppa/disassembler.m"
       {

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

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a20: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned b_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned nulli = addressToPC(MATCH_p);
      unsigned p_31 = (MATCH_w_32_0 & 0x1) /* p_31 at 0 */;
      
      #line 530 "machine/hppa/disassembler.m"
       {

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

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 725 "machine/hppa/disassembler.m"
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


