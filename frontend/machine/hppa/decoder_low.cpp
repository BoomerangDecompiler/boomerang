#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 2 "machine/hppa/decoder_low.m"
/*==============================================================================
 * FILE:        decoder_low.m
 * OVERVIEW:    Low level New Jersey Machine Code Toolkit match file for the
 *              HP Pa/risc architecture (basically PA/RISC version 1.1)
 *
 * (C) 2000-01 The University of Queensland, BT group
 *============================================================================*/

/*
 * $Revision$
 *
 * 30 Nov 00 - Simon: Created
 * 22 Mar 01 - Simon: fixed low_sign_ext problem with ADDI
 * 04 May 01 - Mike: c_cc -> c_c for consistency; fixed a problem with c_null
 *              that seems to be a result of the merge
 * 04 May 01 - Mike: Generate RTLs now instead of strings
 * 19 Jul 01 - Simon: Updated integer/float loads/stores and LDO
 * 23 Jul 01 - Mike: Added "not_used" to quelch annoying warnings
 * 25 Jul 01 - Simon: Added shift, extract, deposit
 * 07 Aug 01 - Simon: Changed iloads and istores to take more parameters, so
 *              that the addressing mode details are explicit in the SSL now
 * 10 Aug 01 - Simon: Moved decoder_low.m:c_bit() to decoder.m:dis_c_bit()
 * 13 Aug 01 - Mike: Provide address with "Undecoded instruction" message
 * 22 Aug 01 - Mike: Fixed a warning with "notused(nulli)"
 */


#include "global.h"
#include "decoder.h"
#include "hppa-names.h"
#include "rtl.h"

void c_null(ADDRESS hostpc, char **garble);
unsigned long c_wcr(ADDRESS hostpc, char **garble);
bool c_c_n(ADDRESS hostpc);
void addr(ADDRESS hostpc);

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

bool c_c_n(ADDRESS hostpc)
{
    bool result = true;


#line 60 "machine/hppa/decoder_low.m"
{ 
  dword MATCH_p = 
    
    #line 60 "machine/hppa/decoder_low.m"
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
          goto MATCH_label_e0; break;
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
                  goto MATCH_label_e2;  /*opt-block+*/
                else 
                  goto MATCH_label_e1;  /*opt-block+*/
                
                break;
              case 1: case 2: case 3: case 6: case 18: case 32: case 33: 
              case 35: case 36: case 37: case 44: case 45: case 49: case 50: 
              case 53: case 54: case 55: case 61: case 62: case 63: 
                goto MATCH_label_e0; break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 6 & 0x3f) -- ext6_20 at 0 --*/ 
          break;
        case 32: case 33: case 39: case 40: case 41: 
          goto MATCH_label_e1; break;
        case 34: case 35: case 42: case 43: case 47: 
          goto MATCH_label_e2; break;
        case 36: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ == 1) 
            goto MATCH_label_e2;  /*opt-block+*/
          else 
            goto MATCH_label_e1;  /*opt-block+*/
          
          break;
        case 37: case 44: case 45: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ == 1 && 
            (0 <= (MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ && 
            (MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ < 2)) 
            goto MATCH_label_e2;  /*opt-block+*/
          else 
            goto MATCH_label_e1;  /*opt-block+*/
          
          break;
        case 50: case 51: 
          if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) 
            goto MATCH_label_e2;  /*opt-block+*/
          else 
            goto MATCH_label_e1;  /*opt-block+*/
          
          break;
        case 52: 
          if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 0 || 
              2 <= (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ && 
              (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ < 8 || 
              (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 1 && 
              (MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 0 && 
              (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 1) 
              goto MATCH_label_e0;  /*opt-block+*/
            else 
              goto MATCH_label_e2;  /*opt-block+*/ /*opt-block+*/
          else 
            if ((MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 0 && 
              (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 1) 
              goto MATCH_label_e0;  /*opt-block+*/
            else 
              goto MATCH_label_e1;  /*opt-block+*/ /*opt-block+*/
          break;
        case 53: 
          if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 0 || 
              2 <= (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ && 
              (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ < 8) 
              goto MATCH_label_e0;  /*opt-block+*/
            else 
              goto MATCH_label_e2;  /*opt-block+*/ /*opt-block+*/
          else 
            goto MATCH_label_e1;  /*opt-block+*/
          break;
        case 59: 
          if ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */ == 1) 
              goto MATCH_label_e2;  /*opt-block+*/
            else 
              goto MATCH_label_e0;  /*opt-block+*/ /*opt-block+*/
          else 
            goto MATCH_label_e1;  /*opt-block+*/
          break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_e; 
  
  MATCH_label_e0: (void)0; /*placeholder for label*/ 
    assert(0);  /* no match */ 
    goto MATCH_finished_e; 
    
  MATCH_label_e1: (void)0; /*placeholder for label*/ 
    
    #line 61 "machine/hppa/decoder_low.m"
     { result = true; }

    
     
    goto MATCH_finished_e; 
    
  MATCH_label_e2: (void)0; /*placeholder for label*/ 
    
    #line 62 "machine/hppa/decoder_low.m"
        { result = false; }

    
     
    goto MATCH_finished_e; 
    
  MATCH_finished_e: (void)0; /*placeholder for label*/
  
}

#line 65 "machine/hppa/decoder_low.m"
    return result;
}

SemStr* NJMCDecoder::c_c(ADDRESS hostpc, int &cond)
{
  static const char *c_c_names[] = {
                "c_c_no", "c_c_eq", "c_c_l", "c_c_le", "c_c_ul", "c_c_ule", "c_c_sv",
                "c_c_od", "c_c_yes", "c_c_ne", "c_c_ge", "c_c_g", "c_c_uge", "c_c_ug",
                "c_c_nsv", "c_c_ev"
        };
  static const int cmpib_codes[] = { 4, 1, 2, 3, 12, 9, 10, 11 };
  static const int sep_codes[] = { 0, 1, 2, 7, 8, 9, 10, 15 };


#line 76 "machine/hppa/decoder_low.m"
{ 
  dword MATCH_p = 
    
    #line 76 "machine/hppa/decoder_low.m"
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
        case 58: case 60: case 61: case 62: case 63: 
          goto MATCH_label_d0; break;
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
                  unsigned c3 = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
                  unsigned cmplt = addressToPC(MATCH_p);
                  
                  #line 81 "machine/hppa/decoder_low.m"
                   {

                              cond = c3 + (c_c_n(cmplt)?0:8); 

                          }

                  
                  
                  
                } /*opt-block*//*opt-block+*/
                else 
                  goto MATCH_label_d1;  /*opt-block+*/
                
                break;
              case 1: case 2: case 3: case 6: case 18: case 32: case 33: 
              case 35: case 36: case 37: case 44: case 45: case 49: case 50: 
              case 53: case 54: case 55: case 61: case 62: case 63: 
                goto MATCH_label_d0; break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 6 & 0x3f) -- ext6_20 at 0 --*/ 
          break;
        case 10: 
          
          #line 84 "machine/hppa/decoder_low.m"
           {

                      cond = 0;

                  }

          
          
          
          break;
        case 32: case 33: case 34: case 35: 
          { 
            unsigned c3 = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
            unsigned cmplt = addressToPC(MATCH_p);
            
            #line 90 "machine/hppa/decoder_low.m"
             {

                        cond = c3 + (c_c_n(cmplt)?0:8); 

                    }

            
            
            
          }
          
          break;
        case 36: 
          goto MATCH_label_d1; break;
        case 37: case 44: case 45: 
          goto MATCH_label_d1; break;
        case 39: case 47: 
          { 
            unsigned c3 = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
            unsigned cmplt = addressToPC(MATCH_p);
            
            #line 93 "machine/hppa/decoder_low.m"
             {

                        cond = c3 + (c_c_n(cmplt)?0:8); 

                    }

            
            
            
          }
          
          break;
        case 40: case 41: case 42: case 43: 
          goto MATCH_label_d1; break;
        case 48: case 49: 
          if ((MATCH_w_32_0 >> 13 & 0x1) /* d_18 at 0 */ == 1) { 
            unsigned c = (MATCH_w_32_0 >> 15 & 0x1) /* c_16 at 0 */;
            
            #line 102 "machine/hppa/decoder_low.m"
             {

                        cond = 1 + (c?0:8); 

                    }

            
            
            
          } /*opt-block*//*opt-block+*/
          else { 
            unsigned c = (MATCH_w_32_0 >> 15 & 0x1) /* c_16 at 0 */;
            
            #line 99 "machine/hppa/decoder_low.m"
             {

                        cond = 1 + (c?0:8); 

                    }

            
            
            
          } /*opt-block*//*opt-block+*/
          
          break;
        case 50: case 51: 
          goto MATCH_label_d2; break;
        case 52: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 0 && 
            (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 1) 
            goto MATCH_label_d0;  /*opt-block+*/
          else 
            goto MATCH_label_d2;  /*opt-block+*/
          
          break;
        case 53: 
          goto MATCH_label_d2; break;
        case 59: 
          { 
            unsigned c3 = 
              ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ << 2) + 
              (MATCH_w_32_0 >> 13 & 0x3) /* c2_17 at 0 */;
            
            #line 96 "machine/hppa/decoder_low.m"
             {

                        cond = cmpib_codes[c3];

                    }

            
            
            
          }
          
          break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_d; 
  
  MATCH_label_d0: (void)0; /*placeholder for label*/ 
    
    #line 105 "machine/hppa/decoder_low.m"
    {

                cond = 0;

            }

    
     
    goto MATCH_finished_d; 
    
  MATCH_label_d1: (void)0; /*placeholder for label*/ 
    { 
      unsigned c3 = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
      unsigned cmplt = addressToPC(MATCH_p);
      
      #line 78 "machine/hppa/decoder_low.m"
       {

                  cond = c3 + (c_c_n(cmplt)?0:8); 

              }

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d2: (void)0; /*placeholder for label*/ 
    { 
      unsigned c3_16 = 
        ((MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ << 2) + 
        (MATCH_w_32_0 >> 13 & 0x3) /* c2_17 at 0 */;
      
      #line 87 "machine/hppa/decoder_low.m"
       {

                  cond = sep_codes[c3_16];

              }

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_finished_d: (void)0; /*placeholder for label*/
  
}

#line 109 "machine/hppa/decoder_low.m"
  return instantiateNamedParam(c_c_names[cond]);
}

unsigned long c_wcr(ADDRESS hostpc, char **garble)
{
#if 0
  unsigned long regl;


#line 115 "machine/hppa/decoder_low.m"
{ 
  dword MATCH_p = 
    
    #line 115 "machine/hppa/decoder_low.m"
    hostpc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 5 & 0xff) /* ext8_19 at 0 */ == 69) 
      if ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 0) 
        if ((MATCH_w_32_0 >> 14 & 0x1) /* ext_17 at 0 */ == 1) 
          
          #line 120 "machine/hppa/decoder_low.m"
           {

                  *garble += sprintf(*garble,".w");

                  regl = 11;

              }

          
           /*opt-block+*/
        else { 
          unsigned r_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
          
          #line 117 "machine/hppa/decoder_low.m"
           {

                  regl = r_06;

              }

          
          
          
        } /*opt-block*//*opt-block+*/ /*opt-block+*/
      else 
        goto MATCH_label_c0;  /*opt-block+*/ 
    else 
      goto MATCH_label_c0;  /*opt-block+*/
    
  }goto MATCH_finished_c; 
  
  MATCH_label_c0: (void)0; /*placeholder for label*/ 
    
    #line 124 "machine/hppa/decoder_low.m"
    {

            regl = 0;

            //sprintf("#c_WCR%08X#", getDword(hostpc));

        }

    
     
    goto MATCH_finished_c; 
    
  MATCH_finished_c: (void)0; /*placeholder for label*/
  
}

#line 129 "machine/hppa/decoder_low.m"
  return regl;
#else
    return 0;
#endif
}

void c_null(ADDRESS hostpc, char **garble)
{
#if 0


#line 137 "machine/hppa/decoder_low.m"
{ 
  dword MATCH_p = 
    
    #line 137 "machine/hppa/decoder_low.m"
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
          
          #line 144 "machine/hppa/decoder_low.m"
          {

                  //printf("#c_NULL%08X#", getDword(hostpc));

              }

          
          
          
          break;
        case 32: case 33: case 34: case 35: case 39: case 40: case 41: 
        case 42: case 43: case 47: case 48: case 49: case 50: case 51: 
        case 56: case 57: case 58: case 59: 
          if ((MATCH_w_32_0 >> 1 & 0x1) /* n_30 at 0 */ == 1) 
            
            #line 141 "machine/hppa/decoder_low.m"
             {

                    *garble += sprintf(*garble, ".n");

                }

            
             /*opt-block+*/
          else 
            
            #line 138 "machine/hppa/decoder_low.m"
             {

                }

            
             /*opt-block+*/
          
          break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_b; 
  
  MATCH_finished_b: (void)0; /*placeholder for label*/
  
}

#line 148 "machine/hppa/decoder_low.m"
#endif
}

/*==============================================================================
 * FUNCTION:       NJMCDecoder::decodeLowLevelInstruction
 * OVERVIEW:       Decodes a machine instruction and returns an instantiated
 *                  list of RTs.
 * NOTE:           A side effect of decoding the completers is that there may
 *                  be some semantics added to members preInstSem or postInstSem
 *                  This is made part of the final RTL in decoder.m
 * PARAMETERS:     hostPC - the address of the pc in the loaded Elf object
 *                 pc - the virtual address of the pc
 *                 result - a reference parameter that has a fields for the
 *                  number of bytes decoded, their validity, etc
 * RETURNS:        the instantiated list of RTs
 *============================================================================*/
list<RT*>* NJMCDecoder::decodeLowLevelInstruction (ADDRESS hostPC, ADDRESS pc,
                        DecodeResult& result)
{
    ADDRESS nextPC; 
  
    list<RT*>* RTs = NULL;
    int condvalue;


#line 170 "machine/hppa/decoder_low.m"
{ 
  dword MATCH_p = 
    
    #line 170 "machine/hppa/decoder_low.m"
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
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "RFI", 
    (char *)0, (char *)0, (char *)0, (char *)0, "RFI.r", (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, "SSM", (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "RSM", (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "MFIA", 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, "MTCTL", "MTSM", (char *)0, 
    (char *)0, "MTSARCM", 
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
  static char *MATCH_name_p_20_88[] = {"VSHD", "SHD", };
  static char *MATCH_name_se_21_89[] = {"VEXTRU", "VEXTRS", };
  static char *MATCH_name_se_21_90[] = {"EXTRU", "EXTRS", };
  static char *MATCH_name_se_21_91[] = {"ZVDEP", "VDEP", };
  static char *MATCH_name_se_21_92[] = {"ZDEP", "DEP", };
  static char *MATCH_name_se_21_93[] = {"ZVDEPI", "VDEPI", };
  static char *MATCH_name_se_21_94[] = {"ZDEPI", "DEPI", };
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    
      switch((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */) {
        case 0: 
          if (33 <= (MATCH_w_32_0 >> 5 & 0xff) /* ext8_19 at 0 */ && 
            (MATCH_w_32_0 >> 5 & 0xff) /* ext8_19 at 0 */ < 69 || 
            116 <= (MATCH_w_32_0 >> 5 & 0xff) /* ext8_19 at 0 */ && 
            (MATCH_w_32_0 >> 5 & 0xff) /* ext8_19 at 0 */ < 165 || 
            199 <= (MATCH_w_32_0 >> 5 & 0xff) /* ext8_19 at 0 */ && 
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
                  nextPC = 4 + MATCH_p; 
                  
                  #line 281 "machine/hppa/decoder_low.m"
                   {

                              RTs = instantiate(pc, name, dis_Num(im5_27), dis_Num(im13_06));

                          }

                  
                  
                  
                }
                
                break;
              case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: 
              case 9: case 10: case 11: case 12: case 13: case 14: case 15: 
              case 16: case 17: case 18: case 19: case 20: case 21: case 22: 
              case 23: case 24: case 25: case 26: case 27: case 28: case 29: 
              case 30: case 31: case 70: case 71: case 72: case 73: case 74: 
              case 75: case 76: case 77: case 78: case 79: case 80: case 81: 
              case 82: case 83: case 84: case 85: case 86: case 87: case 88: 
              case 89: case 90: case 91: case 92: case 93: case 94: case 95: 
              case 97: case 98: case 99: case 100: case 102: case 103: 
              case 104: case 105: case 106: case 108: case 109: case 110: 
              case 111: case 112: case 113: case 114: case 166: case 167: 
              case 168: case 169: case 170: case 171: case 172: case 173: 
              case 174: case 175: case 176: case 177: case 178: case 179: 
              case 180: case 181: case 182: case 183: case 184: case 185: 
              case 186: case 187: case 188: case 189: case 190: case 191: 
              case 192: case 193: case 196: case 197: 
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
              case 69: 
                MATCH_name = MATCH_name_ext_17_2[(MATCH_w_32_0 >> 14 & 0x1) 
                      /* ext_17 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned cmplt = addressToPC(MATCH_p);
                  unsigned t_27 = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 293 "machine/hppa/decoder_low.m"
                   {

                              RTs = instantiate(pc, name, dis_c_wcr(cmplt), dis_Reg(t_27));

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
                  nextPC = 4 + MATCH_p; 
                  
                  #line 284 "machine/hppa/decoder_low.m"
                   {

                              RTs = instantiate(pc, name, dis_Num(im10_06), dis_Reg(t_27));

                          }

                  
                  
                  
                }
                
                break;
              case 165: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned t_27 = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 299 "machine/hppa/decoder_low.m"
                   {

                              RTs = instantiate(pc, name, dis_Reg(t_27));

                          }

                  // Floating point instructions. Note that the floating point format is being

                  // passed as an ss inf the form of an integer constant (using dis_Num())

                  
                  
                  
                }
                
                break;
              case 194: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned ct_06 = 
                    (MATCH_w_32_0 >> 21 & 0x1f) /* cr_06 at 0 */;
                  unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 296 "machine/hppa/decoder_low.m"
                   {

                              RTs = instantiate(pc, name, dis_Reg(r_11), dis_ct(ct_06));

                          }

                  
                  
                  
                }
                
                break;
              case 195: case 198: 
                MATCH_name = MATCH_name_ext8_19_0[(MATCH_w_32_0 >> 5 & 0xff) 
                      /* ext8_19 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 290 "machine/hppa/decoder_low.m"
                   {

                              RTs = instantiate(pc, name, dis_Reg(r_11));

                          }

                  
                  
                  
                }
                
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 5 & 0xff) -- ext8_19 at 0 --*/ 
          break;
        case 1: case 4: case 5: case 6: case 7: case 15: case 20: case 21: 
        case 22: case 23: case 28: case 29: case 30: case 31: case 32: 
        case 33: case 34: case 35: case 38: case 39: case 40: case 41: 
        case 42: case 43: case 46: case 47: case 48: case 49: case 50: 
        case 51: case 54: case 55: case 56: case 57: case 59: case 60: 
        case 61: case 62: case 63: 
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
                MATCH_name = "OR"; goto MATCH_label_a2; break;
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
            nextPC = 4 + MATCH_p; 
            
            #line 185 "machine/hppa/decoder_low.m"
             {

                        RTs = instantiate(pc, name, dis_Num(imm21), dis_Reg(t_06));

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
            nextPC = 4 + MATCH_p; 
            
            #line 182 "machine/hppa/decoder_low.m"
             {

                        RTs = instantiate(pc, name, dis_Num(imm21), dis_Reg(r_06));

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
                    unsigned rf = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                    unsigned tf = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                    nextPC = 4 + MATCH_p; 
                    
                    #line 304 "machine/hppa/decoder_low.m"
                     {

                                RTs = instantiate(pc, name, dis_Num(fmt), dis_Freg(rf, fmt),

                                    dis_Freg(tf, fmt));

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
                    unsigned rf = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                    unsigned sf = (MATCH_w_32_0 >> 11 & 0x3) /* sf_19 at 0 */;
                    unsigned tf = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                    nextPC = 4 + MATCH_p; 
                    
                    #line 308 "machine/hppa/decoder_low.m"
                     {

                                RTs = instantiate(pc, name, dis_Num(sf), dis_Num(df),

                                    dis_Freg(rf, sf), dis_Freg(tf, df));

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
            nextPC = 4 + MATCH_p; 
            
            #line 221 "machine/hppa/decoder_low.m"
             {

                        RTs = instantiate(pc, name, dis_Num(ldisp), dis_Reg(b), dis_Reg(t));

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
                    unsigned rf = 
                      ((MATCH_w_32_0 >> 7 & 0x1) /* r1_24 at 0 */ << 5) + 
                      (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                    unsigned sf = (MATCH_w_32_0 >> 11 & 0x3) /* sf_19 at 0 */;
                    unsigned tf = 
                      ((MATCH_w_32_0 >> 6 & 0x1) /* t_25 at 0 */ << 5) + 
                      (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                    nextPC = 4 + MATCH_p; 
                    
                    #line 308 "machine/hppa/decoder_low.m"
                     {

                                RTs = instantiate(pc, name, dis_Num(sf), dis_Num(df),

                                    dis_Freg(rf, sf), dis_Freg(tf, df));

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
                        unsigned fr1 = 
                          ((MATCH_w_32_0 >> 7 & 0x1) /* r1_24 at 0 */ << 5) + 
                          (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                        unsigned fr2 = 
                          ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ << 5) + 
                          (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                        unsigned frt = 
                          ((MATCH_w_32_0 >> 6 & 0x1) /* t_25 at 0 */ << 5) + 
                          (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 320 "machine/hppa/decoder_low.m"
                         {

                                    // This instruction has fixed register sizes

                                    RTs = instantiate(pc, "XMPYU", dis_Freg(fr1, 0), dis_Freg(fr2, 0),

                                        dis_Freg(frt, 1));

                                }

                        //      | LDSID(s2_16,b_06,t_27)[name] => {

                        //      }

                        //      | MTSP(r_11,sr)[name] => {

                        //      }

                        //      | MFSP(sr,t_27)[name] => {

                        //      }

                        
                        
                        
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
            unsigned r_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
            unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
            unsigned xd = addressToPC(MATCH_p);
            nextPC = 4 + MATCH_p; 
            
            #line 213 "machine/hppa/decoder_low.m"
             {

                        RTs = instantiate(pc, name, dis_c_addr(c_addr), dis_xd(xd),

                            dis_Sreg(s), dis_Reg(b), dis_Reg(r_11));

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
            nextPC = 4 + MATCH_p; 
            
            #line 217 "machine/hppa/decoder_low.m"
             {

                        RTs = instantiate(pc, name, dis_c_addr(c_addr), dis_Reg(r_11),

                            dis_xd(xd), dis_Sreg(s), dis_Reg(b));

                    }

            
            
            
          }
          
          break;
        case 36: 
          MATCH_name = "CMPICLR"; goto MATCH_label_a13; break;
        case 37: 
          if ((MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ == 1 && 
            (0 <= (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ && 
            (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ < 2)) { 
            MATCH_name = "SUBI.v"; 
            goto MATCH_label_a13; 
            
          } /*opt-block*/
          else { MATCH_name = "SUBI"; goto MATCH_label_a13; } /*opt-block*/
          
          break;
        case 44: 
          if ((MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ == 1 && 
            (0 <= (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ && 
            (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ < 2)) { 
            MATCH_name = "ADDI.t.v"; 
            goto MATCH_label_a13; 
            
          } /*opt-block*/
          else { MATCH_name = "ADDI.t"; goto MATCH_label_a13; } /*opt-block*/
          
          break;
        case 45: 
          if ((MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ == 1 && 
            (0 <= (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ && 
            (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ < 2)) { 
            MATCH_name = "ADDI.v"; 
            goto MATCH_label_a13; 
            
          } /*opt-block*/
          else { MATCH_name = "ADDI"; goto MATCH_label_a13; } /*opt-block*/
          
          break;
        case 52: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ == 1 && 
              (0 <= (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ && 
              (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ < 2)) { 
              MATCH_name = MATCH_name_se_21_90[(MATCH_w_32_0 >> 10 & 0x1) 
                    /* se_21 at 0 */]; 
              { 
                char *name = MATCH_name;
                unsigned c = addressToPC(MATCH_p);
                unsigned len = 32 - (MATCH_w_32_0 & 0x1f) /* clen5_27 at 0 */;
                unsigned p = (MATCH_w_32_0 >> 5 & 0x1f) /* pos5_22 at 0 */;
                unsigned r = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                unsigned t = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
                nextPC = 4 + MATCH_p; 
                
                #line 236 "machine/hppa/decoder_low.m"
                 {

                            RTs = instantiate(pc, name, dis_Reg(r), dis_Num(p), dis_Num(len),

                                dis_Reg(t), c_c(c, condvalue));

                        }

                
                
                
              }
              
            } /*opt-block*/
            else { 
              MATCH_name = MATCH_name_se_21_89[(MATCH_w_32_0 >> 10 & 0x1) 
                    /* se_21 at 0 */]; 
              { 
                char *name = MATCH_name;
                unsigned c = addressToPC(MATCH_p);
                unsigned len = 32 - (MATCH_w_32_0 & 0x1f) /* clen5_27 at 0 */;
                unsigned r = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                unsigned t = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
                nextPC = 4 + MATCH_p; 
                
                #line 232 "machine/hppa/decoder_low.m"
                 {

                            RTs = instantiate(pc, name, dis_Reg(r), dis_Num(len), dis_Reg(t),

                                c_c(c, condvalue));

                        }

                
                
                
              }
              
            } /*opt-block*/ /*opt-block+*/
          else 
            if ((MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 1) 
              goto MATCH_label_a0;  /*opt-block+*/
            else 
              if ((MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ == 1) { 
                MATCH_name = MATCH_name_p_20_88[(MATCH_w_32_0 >> 11 & 0x1) 
                      /* p_20 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned c = addressToPC(MATCH_p);
                  unsigned p = 
                    31 - (MATCH_w_32_0 >> 5 & 0x1f) /* pos5_22 at 0 */;
                  unsigned r1 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                  unsigned r2 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                  unsigned t = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 228 "machine/hppa/decoder_low.m"
                   {

                              RTs = instantiate(pc, name, dis_Reg(r1), dis_Reg(r2), dis_Num(p),

                                  dis_Reg(t), c_c(c, condvalue));

                          }

                  
                  
                  
                }
                
              } /*opt-block*/
              else { 
                MATCH_name = MATCH_name_p_20_88[(MATCH_w_32_0 >> 11 & 0x1) 
                      /* p_20 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned c = addressToPC(MATCH_p);
                  unsigned r1 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                  unsigned r2 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
                  unsigned t = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 224 "machine/hppa/decoder_low.m"
                   {

                              RTs = instantiate(pc, name, dis_Reg(r1), dis_Reg(r2), dis_Reg(t),

                                  c_c(c, condvalue));

                          }

                  
                  
                  
                }
                
              } /*opt-block*/ /*opt-block+*/ 
          break;
        case 53: 
          if ((MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 1) 
            if ((MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ == 1 && 
              (0 <= (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ && 
              (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ < 2)) { 
              MATCH_name = MATCH_name_se_21_94[(MATCH_w_32_0 >> 10 & 0x1) 
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
                nextPC = 4 + MATCH_p; 
                
                #line 252 "machine/hppa/decoder_low.m"
                 {

                            RTs = instantiate(pc, name, dis_Num(i), dis_Num(p), dis_Num(len),

                                dis_Reg(t), c_c(c, condvalue));

                        }

                
                
                
              }
              
            } /*opt-block*/
            else { 
              MATCH_name = MATCH_name_se_21_93[(MATCH_w_32_0 >> 10 & 0x1) 
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
                nextPC = 4 + MATCH_p; 
                
                #line 248 "machine/hppa/decoder_low.m"
                 {

                            RTs = instantiate(pc, name, dis_Num(i), dis_Num(len), dis_Reg(t),

                                c_c(c, condvalue));

                        }

                
                
                
              }
              
            } /*opt-block*/ /*opt-block+*/
          else 
            if ((MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ == 1 && 
              (0 <= (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ && 
              (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ < 2)) { 
              MATCH_name = MATCH_name_se_21_92[(MATCH_w_32_0 >> 10 & 0x1) 
                    /* se_21 at 0 */]; 
              { 
                char *name = MATCH_name;
                unsigned c = addressToPC(MATCH_p);
                unsigned len = 32 - (MATCH_w_32_0 & 0x1f) /* clen5_27 at 0 */;
                unsigned p = 
                  31 - (MATCH_w_32_0 >> 5 & 0x1f) /* pos5_22 at 0 */;
                unsigned r = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                unsigned t = (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
                nextPC = 4 + MATCH_p; 
                
                #line 244 "machine/hppa/decoder_low.m"
                 {

                            RTs = instantiate(pc, name, dis_Reg(r), dis_Num(p), dis_Num(len),

                                dis_Reg(t), c_c(c, condvalue));

                        }

                
                
                
              }
              
            } /*opt-block*/
            else { 
              MATCH_name = MATCH_name_se_21_91[(MATCH_w_32_0 >> 10 & 0x1) 
                    /* se_21 at 0 */]; 
              { 
                char *name = MATCH_name;
                unsigned c = addressToPC(MATCH_p);
                unsigned len = 32 - (MATCH_w_32_0 & 0x1f) /* clen5_27 at 0 */;
                unsigned r = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
                unsigned t = (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
                nextPC = 4 + MATCH_p; 
                
                #line 240 "machine/hppa/decoder_low.m"
                 {

                            RTs = instantiate(pc, name, dis_Reg(r), dis_Num(len), dis_Reg(t),

                                c_c(c, condvalue));

                        }

                
                
                
              }
              
            } /*opt-block*/ /*opt-block+*/
          break;
        case 58: 
          
            switch((MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */) {
              case 0: 
                MATCH_name = "BL"; goto MATCH_label_a14; break;
              case 1: 
                MATCH_name = "GATE"; goto MATCH_label_a14; break;
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
                    nextPC = 4 + MATCH_p; 
                    
                    #line 266 "machine/hppa/decoder_low.m"
                     {

                                /* BLR,n x,t */

                                RTs = instantiate(pc, name, dis_Reg(x_11), dis_Reg(t_06));

                                not_used(nulli);

                            }

                    
                    
                    
                  }
                  
                } /*opt-block*/
                
                break;
              case 3: 
                goto MATCH_label_a0; break;
              case 4: 
                MATCH_name = "BL.PUSH"; goto MATCH_label_a14; break;
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
                  nextPC = 4 + MATCH_p; 
                  
                  #line 261 "machine/hppa/decoder_low.m"
                   {

                              /* BL.LONG cmplt,n  target,2) */

                              RTs = instantiate(pc, name, dis_Num(ubr_target));

                              not_used(nulli);

                          }

                  
                  
                  
                }
                
                break;
              case 6: 
                if ((MATCH_w_32_0 >> 12 & 0x1) /* ve_19 at 0 */ == 1 && 
                  (0 <= (MATCH_w_32_0 >> 1 & 0x1) /* n_30 at 0 */ && 
                  (MATCH_w_32_0 >> 1 & 0x1) /* n_30 at 0 */ < 2)) { 
                  MATCH_name = "BVE"; 
                  goto MATCH_label_a15; 
                  
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
                    nextPC = 4 + MATCH_p; 
                    
                    #line 271 "machine/hppa/decoder_low.m"
                     {

                                /* BV,n x_11(b_06) */

                                RTs = instantiate(pc, name, dis_Reg(x_11), dis_Reg(b_06));

                                not_used(nulli);

                            }

                    
                    
                    
                  }
                  
                } /*opt-block*/
                
                break;
              case 7: 
                if ((MATCH_w_32_0 >> 12 & 0x1) /* ve_19 at 0 */ == 0) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else { 
                  MATCH_name = "BVE.l"; 
                  goto MATCH_label_a15; 
                  
                } /*opt-block*/
                
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 13 & 0x7) -- ext3_16 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 26 & 0x3f) -- op at 0 --*/ 
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 331 "machine/hppa/decoder_low.m"
      {

      		    //RTs = NULL;

                  result.valid = false;

      cout << "Undecoded instruction " << hex << *(int*)hostPC << " at " << pc << " (opcode " << ((*(unsigned*)hostPC) >> 26) << ")\n";

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a1: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      nextPC = 4 + MATCH_p; 
      
      #line 287 "machine/hppa/decoder_low.m"
       {

                  RTs = instantiate(pc, name);

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
      nextPC = 4 + MATCH_p; 
      
      #line 172 "machine/hppa/decoder_low.m"
       {

                  /*  Arith,cc_16   r_11, r_06, t_27 */

                  RTs = instantiate(pc, name, dis_Reg(r_11), dis_Reg(r_06),

                    dis_Reg(t_27),c_c(cmplt, condvalue));

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
      nextPC = 4 + MATCH_p; 
      
      #line 188 "machine/hppa/decoder_low.m"
       {

                  RTs = instantiate(pc, name, dis_c_addr(c_addr),

      				dis_xd(xd), dis_Sreg(s),

                      dis_Reg(b), dis_Reg(t_27));

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
      nextPC = 4 + MATCH_p; 
      
      #line 193 "machine/hppa/decoder_low.m"
       {

                  RTs = instantiate(pc, name, dis_c_addr(c_addr), dis_Reg(r_11),

                      dis_xd(xd), dis_Sreg(s), dis_Reg(b));

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
      nextPC = 4 + MATCH_p; 
      
      #line 197 "machine/hppa/decoder_low.m"
       {

                  RTs = instantiate(pc, name, dis_c_addr(c_addr), dis_xd(xd),

                      dis_Sreg(s), dis_Reg(b), dis_Freg(t_27, 0));

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a6: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned c_addr = addressToPC(MATCH_p);
      unsigned r_27 = 
        ((MATCH_w_32_0 >> 6 & 0x1) /* r_25 at 0 */ << 5) + 
        (MATCH_w_32_0 & 0x1f) /* r_27 at 0 */;
      unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned xd = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 201 "machine/hppa/decoder_low.m"
       {

                  RTs = instantiate(pc, name, dis_c_addr(c_addr), dis_Freg(r_27, 0),

                      dis_xd(xd), dis_Sreg(s), dis_Reg(b));

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
      nextPC = 4 + MATCH_p; 
      
      #line 205 "machine/hppa/decoder_low.m"
       {

                  RTs = instantiate(pc, name, dis_c_addr(c_addr), dis_xd(xd),

                      dis_Sreg(s), dis_Reg(b), dis_Freg(t_27, 1));

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a8: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned c_addr = addressToPC(MATCH_p);
      unsigned r_27 = (MATCH_w_32_0 & 0x1f) /* r_27 at 0 */;
      unsigned s = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned xd = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 209 "machine/hppa/decoder_low.m"
       {

                  RTs = instantiate(pc, name, dis_c_addr(c_addr), dis_Freg(r_27, 1),

                      dis_xd(xd), dis_Sreg(s), dis_Reg(b));

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a9: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned df = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      unsigned rf = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned sf = (MATCH_w_32_0 >> 11 & 0x3) /* fmt_19 at 0 */;
      unsigned tf = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 312 "machine/hppa/decoder_low.m"
       {

                  RTs = instantiate(pc, name, dis_Num(sf), dis_Num(df),

                      dis_Freg(rf, sf), dis_Freg(tf, df));

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a10: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned fmt = (MATCH_w_32_0 >> 11 & 0x3) /* fmt_19 at 0 */;
      unsigned fr1 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned fr2 = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned frt = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 316 "machine/hppa/decoder_low.m"
       {

                  RTs = instantiate(pc, name, dis_Num(fmt), dis_Freg(fr1, fmt),

                      dis_Freg(fr2, fmt), dis_Freg(frt, fmt));

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a11: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned df = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      unsigned rf = 
        ((MATCH_w_32_0 >> 7 & 0x1) /* r1_24 at 0 */ << 5) + 
        (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned sf = (MATCH_w_32_0 >> 11 & 0x1) /* f_20 at 0 */;
      unsigned tf = 
        ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ << 5) + 
        (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 312 "machine/hppa/decoder_low.m"
       {

                  RTs = instantiate(pc, name, dis_Num(sf), dis_Num(df),

                      dis_Freg(rf, sf), dis_Freg(tf, df));

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a12: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned fmt = (MATCH_w_32_0 >> 11 & 0x1) /* f_20 at 0 */;
      unsigned fr1 = 
        ((MATCH_w_32_0 >> 7 & 0x1) /* r1_24 at 0 */ << 5) + 
        (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned fr2 = 
        ((MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ << 5) + 
        (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned frt = 
        ((MATCH_w_32_0 >> 6 & 0x1) /* t_25 at 0 */ << 5) + 
        (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 316 "machine/hppa/decoder_low.m"
       {

                  RTs = instantiate(pc, name, dis_Num(fmt), dis_Freg(fr1, fmt),

                      dis_Freg(fr2, fmt), dis_Freg(frt, fmt));

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a13: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned cmplt = addressToPC(MATCH_p);
      unsigned imm11 = 
        (sign_extend((MATCH_w_32_0 & 0x1) /* i_31 at 0 */, 1) << 10) + 
        (MATCH_w_32_0 >> 1 & 0x3ff) /* im10_21 at 0 */;
      unsigned r_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned t_11 = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 177 "machine/hppa/decoder_low.m"
       {

                  /* arith_imm,cc_16 imm11!,r_06,t_11 */

                  RTs = instantiate(pc, name, dis_Num(imm11), dis_Reg(r_06),

                    dis_Reg(t_11), c_c(cmplt, condvalue));

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a14: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned nulli = addressToPC(MATCH_p);
      unsigned t_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
      unsigned ubr_target = 
        8 + ((MATCH_w_32_0 >> 3 & 0x3ff) /* w10_19 at 0 */ << 2) + 
        ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 12) + 
        ((MATCH_w_32_0 >> 16 & 0x1f) /* w5_11 at 0 */ << 13) + 
        (sign_extend((MATCH_w_32_0 & 0x1) /* w_31 at 0 */, 1) << 18);
      nextPC = 4 + MATCH_p; 
      
      #line 256 "machine/hppa/decoder_low.m"
       {

                  /* ubranch,cmplt,n  target,t_06) */

                  RTs = instantiate(pc, name, dis_Num(ubr_target), dis_Reg(t_06));

                  not_used(nulli);

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a15: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned b_06 = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned nulli = addressToPC(MATCH_p);
      unsigned p_31 = (MATCH_w_32_0 & 0x1) /* p_31 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 276 "machine/hppa/decoder_low.m"
       {

                  /* BVE.l BVE.lp BVE.p BVE  */

                  RTs = instantiate(pc, name, p_31, dis_Reg(b_06));

                  not_used(nulli);

              }

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 337 "machine/hppa/decoder_low.m"

    result.numBytes = (nextPC - hostPC);
    return RTs;

}

/*
        }
        | LDWl(cmplt, ldisp, s2_16, b_06, t_11)[name] => {
                *garble += sprintf(*garble, "%s", name);
            c_disps(cmplt);
            *garble += sprintf(*garble, "  %d(%s,%s),%s", ldisp, s2_16_names[s2_16], b_06_names[b_06], t_11_names[t_11]);
        
*/


