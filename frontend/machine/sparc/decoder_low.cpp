#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 2 "machine/sparc/decoder_low.m"
/*
 * Copyright (C) 1996-2001, The University of Queensland
 * Copyright (C) 2000, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       decoder_low.m
 * OVERVIEW:   An implementation of the low level decoding method.
 *             This has been factored out to make compilation of .m
 *             more modular and therefore quicker when changes are
 *             only made in one module.
 *============================================================================*/

/*
 * 24 Jan 00 - Mike: Changes to ensure that double and quad registers get the
 *              correct names (e.g. %f2to3, %f4to7)
 * 20 Jul 00 - Cristina: The [numBytes] notation in a match statement in 
 *				version Apr 5 14:56:28 EDT 2000 of the toolkit (mltk)
 *				now returns a pointer to the next instruction to decode 
 *				rather than the number of bytes decoded.  results.numBytes
 *				was fixed accordingly, and numBytes was renamed nextPC. 
 * 03 Apr 01 - Mike: Added DIS_ADDR to "trap" arm (was segfaulting whenever a
 *              trap instruction was speculatively decoded)
 * 12 Apr 01 - Mike: Added FSQRTd and FSQRTq instructions
 * 24 Oct 01 - Mike: Suppressed some "unused variable" warnings
*/

#include "global.h"
#include "decoder.h"
#include "rtl.h"
#include "sparc-names.h"

void unused(int);

/*==============================================================================
 * FUNCTION:       NJMCDecoder::decodeLowLevelInstruction
 * OVERVIEW:       Decodes a machine instruction and returns an instantiated
 *                 list of RTs.
 * PARAMETERS:     hostPC - the address of the pc in the loaded Elf object
 *                 pc - the virtual address of the pc
 *                 result - a reference parameter that has a fields for the
 *                  number of bytes decoded, their validity, etc
 * RETURNS:        the instantiated list of RTs
 *============================================================================*/
list<RT*>* NJMCDecoder::decodeLowLevelInstruction (ADDRESS hostPC, ADDRESS pc,
	DecodeResult& result)
{

	// The list of instantiated RTs.
	list<RT*>* RTs = NULL;

	ADDRESS nextPC;


#line 58 "machine/sparc/decoder_low.m"
{ 
  dword MATCH_p = 
    
    #line 58 "machine/sparc/decoder_low.m"
    hostPC
    ;
  char *MATCH_name;
  static char *MATCH_name_cond_0[] = {
    "BN", "BE", "BLE", "BL", "BLEU", "BCS", "BNEG", "BVS", "BA", "BNE", "BG", 
    "BGE", "BGU", "BCC", "BPOS", "BVC", 
  };
  static char *MATCH_name_cond_1[] = {
    "BN,a", "BE,a", "BLE,a", "BL,a", "BLEU,a", "BCS,a", "BNEG,a", "BVS,a", 
    "BA,a", "BNE,a", "BG,a", "BGE,a", "BGU,a", "BCC,a", "BPOS,a", "BVC,a", 
  };
  static char *MATCH_name_rd_2[] = {
    "NOP", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
    "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
    "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
    "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
  };
  static char *MATCH_name_cond_3[] = {
    "FBN", "FBNE", "FBLG", "FBUL", "FBL", "FBUG", "FBG", "FBU", "FBA", "FBE", 
    "FBUE", "FBGE", "FBUGE", "FBLE", "FBULE", "FBO", 
  };
  static char *MATCH_name_cond_4[] = {
    "FBN,a", "FBNE,a", "FBLG,a", "FBUL,a", "FBL,a", "FBUG,a", "FBG,a", 
    "FBU,a", "FBA,a", "FBE,a", "FBUE,a", "FBGE,a", "FBUGE,a", "FBLE,a", 
    "FBULE,a", "FBO,a", 
  };
  static char *MATCH_name_cond_5[] = {
    "CBN", "CB123", "CB12", "CB13", "CB1", "CB23", "CB2", "CB3", "CBA", 
    "CB0", "CB03", "CB02", "CB023", "CB01", "CB013", "CB012", 
  };
  static char *MATCH_name_cond_6[] = {
    "CBN,a", "CB123,a", "CB12,a", "CB13,a", "CB1,a", "CB23,a", "CB2,a", 
    "CB3,a", "CBA,a", "CB0,a", "CB03,a", "CB02,a", "CB023,a", "CB01,a", 
    "CB013,a", "CB012,a", 
  };
  static char *MATCH_name_rs1_43[] = {
    "RDY", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
    "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
    "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
    "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
  };
  static char *MATCH_name_op3_44[] = {
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "RDPSR", "RDWIM", 
    "RDTBR", 
  };
  static char *MATCH_name_opf_49[] = {
    (char *)0, "FMOVs", (char *)0, (char *)0, (char *)0, "FNEGs", (char *)0, 
    (char *)0, (char *)0, "FABSs", (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, "FSQRTs", "FSQRTd", "FSQRTq", 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, "FADDs", "FADDd", "FADDq", (char *)0, 
    "FSUBs", "FSUBd", "FSUBq", (char *)0, "FMULs", "FMULd", "FMULq", 
    (char *)0, "FDIVs", "FDIVd", "FDIVq", (char *)0, "FCMPs", "FCMPd", 
    "FCMPq", (char *)0, "FCMPEs", "FCMPEd", "FCMPEq", (char *)0, (char *)0, 
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
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, "FiTOs", (char *)0, "FdTOs", 
    "FqTOs", "FiTOd", "FsTOd", (char *)0, "FqTOd", "FiTOq", "FsTOq", "FdTOq", 
    (char *)0, (char *)0, "FsTOi", "FdTOi", "FqTOi", 
  };
  static char *MATCH_name_cond_52[] = {
    "TN", "TE", "TLE", "TL", "TLEU", "TCS", "TNEG", "TVS", "TA", "TNE", "TG", 
    "TGE", "TGU", "TCC", "TPOS", "TVC", 
  };
  static char *MATCH_name_i_54[] = {"LD", "SAVE", };
  static char *MATCH_name_i_58[] = {"LDUB", "RESTORE", };
  static char *MATCH_name_i_59[] = {"LDUH", "LD", };
  static char *MATCH_name_i_60[] = {"LDD", "LDUB", };
  static char *MATCH_name_i_61[] = {"ST", "LDUH", };
  static char *MATCH_name_i_62[] = {"STB", "LDD", };
  static char *MATCH_name_i_63[] = {"STH", "ST", };
  static char *MATCH_name_i_64[] = {"STD", "STB", };
  static char *MATCH_name_i_65[] = {"LDSB", "STH", };
  static char *MATCH_name_i_66[] = {"LDSH", "STD", };
  static char *MATCH_name_i_67[] = {"LDSTUB", "LDSB", };
  static char *MATCH_name_i_68[] = {"SWAP.", "LDSH", };
  static char *MATCH_name_i_69[] = {"LDA", "LDSTUB", };
  static char *MATCH_name_i_70[] = {"LDUBA", "SWAP.", };
  static char *MATCH_name_i_71[] = {"LDUHA", "LDF", };
  static char *MATCH_name_i_72[] = {"LDDA", "LDFSR", };
  static char *MATCH_name_i_73[] = {"STA", "LDDF", };
  static char *MATCH_name_i_74[] = {"STBA", "STF", };
  static char *MATCH_name_i_75[] = {"STHA", "STFSR", };
  static char *MATCH_name_i_76[] = {"STDA", "STDFQ", };
  static char *MATCH_name_i_77[] = {"LDSBA", "STDF", };
  static char *MATCH_name_i_78[] = {"LDSHA", "LDC", };
  static char *MATCH_name_i_79[] = {"LDSTUBA", "LDCSR", };
  static char *MATCH_name_i_80[] = {"SWAPA", "LDDC", };
  static char *MATCH_name_i_81[] = {"LDF", "STC", };
  static char *MATCH_name_i_82[] = {"LDFSR", "STCSR", };
  static char *MATCH_name_i_83[] = {"LDDF", "STDCQ", };
  static char *MATCH_name_i_84[] = {"STF", "STDC", };
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    
      switch((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */) {
        case 0: 
          
            switch((MATCH_w_32_0 >> 22 & 0x7) /* op2 at 0 */) {
              case 0: 
                { 
                  unsigned n = (MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 238 "machine/sparc/decoder_low.m"
                   

                          unused(n);

                  		RTs = NULL;

                          result.valid = false;

                  

                  
                  
                  
                }
                
                break;
              case 1: case 3: case 5: 
                goto MATCH_label_a0; break;
              case 2: 
                if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1 && 
                  (0 <= (MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ && 
                  (MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ < 16)) { 
                  MATCH_name = MATCH_name_cond_1[(MATCH_w_32_0 >> 25 & 0xf) 
                        /* cond at 0 */]; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = MATCH_name_cond_0[(MATCH_w_32_0 >> 25 & 0xf) 
                        /* cond at 0 */]; 
                  goto MATCH_label_a1; 
                  
                } /*opt-block*/
                
                break;
              case 4: 
                if ((MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */ == 0 && 
                  (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0) { 
                  MATCH_name = MATCH_name_rd_2[(MATCH_w_32_0 >> 25 & 0x1f) 
                        /* rd at 0 */]; 
                  { 
                    char *name = MATCH_name;
                    nextPC = 4 + MATCH_p; 
                    
                    #line 61 "machine/sparc/decoder_low.m"
                    

                    		result.type = NOP;

                    		RTs = instantiate(pc,  name);

                    

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a3;  /*opt-block+*/
                
                break;
              case 6: 
                if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1 && 
                  (0 <= (MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ && 
                  (MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ < 16)) { 
                  MATCH_name = MATCH_name_cond_4[(MATCH_w_32_0 >> 25 & 0xf) 
                        /* cond at 0 */]; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = MATCH_name_cond_3[(MATCH_w_32_0 >> 25 & 0xf) 
                        /* cond at 0 */]; 
                  goto MATCH_label_a1; 
                  
                } /*opt-block*/
                
                break;
              case 7: 
                if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1 && 
                  (0 <= (MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ && 
                  (MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ < 16)) { 
                  MATCH_name = MATCH_name_cond_6[(MATCH_w_32_0 >> 25 & 0xf) 
                        /* cond at 0 */]; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = MATCH_name_cond_5[(MATCH_w_32_0 >> 25 & 0xf) 
                        /* cond at 0 */]; 
                  goto MATCH_label_a1; 
                  
                } /*opt-block*/
                
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 22 & 0x7) -- op2 at 0 --*/ 
          break;
        case 1: 
          { 
            unsigned tgt = 
              4 * sign_extend((MATCH_w_32_0 & 0x3fffffff) /* disp30 at 0 */, 
                          30) + addressToPC(MATCH_p);
            nextPC = 4 + MATCH_p; 
            
            #line 165 "machine/sparc/decoder_low.m"
            

            		result.type = SD;

            		RTs = instantiate(pc,  "call__", dis_Num(((int)(tgt-hostPC))>>2));

            

            
            
            
          }
          
          break;
        case 2: 
          
            switch((MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */) {
              case 0: 
                MATCH_name = "ADD"; goto MATCH_label_a4; break;
              case 1: 
                MATCH_name = "AND"; goto MATCH_label_a4; break;
              case 2: 
                MATCH_name = "OR"; goto MATCH_label_a4; break;
              case 3: 
                MATCH_name = "XOR"; goto MATCH_label_a4; break;
              case 4: 
                MATCH_name = "SUB"; goto MATCH_label_a4; break;
              case 5: 
                MATCH_name = "ANDN"; goto MATCH_label_a4; break;
              case 6: 
                MATCH_name = "ORN"; goto MATCH_label_a4; break;
              case 7: 
                MATCH_name = "XNOR"; goto MATCH_label_a4; break;
              case 8: 
                MATCH_name = "ADDX"; goto MATCH_label_a4; break;
              case 9: case 13: case 25: case 29: case 44: case 45: case 46: 
              case 47: case 54: case 55: case 59: case 62: case 63: 
                goto MATCH_label_a0; break;
              case 10: 
                MATCH_name = "UMUL"; goto MATCH_label_a4; break;
              case 11: 
                MATCH_name = "SMUL"; goto MATCH_label_a4; break;
              case 12: 
                MATCH_name = "SUBX"; goto MATCH_label_a4; break;
              case 14: 
                MATCH_name = "UDIV"; goto MATCH_label_a4; break;
              case 15: 
                MATCH_name = "SDIV"; goto MATCH_label_a4; break;
              case 16: 
                MATCH_name = "ADDcc"; goto MATCH_label_a4; break;
              case 17: 
                MATCH_name = "ANDcc"; goto MATCH_label_a4; break;
              case 18: 
                MATCH_name = "ORcc"; goto MATCH_label_a4; break;
              case 19: 
                MATCH_name = "XORcc"; goto MATCH_label_a4; break;
              case 20: 
                MATCH_name = "SUBcc"; goto MATCH_label_a4; break;
              case 21: 
                MATCH_name = "ANDNcc"; goto MATCH_label_a4; break;
              case 22: 
                MATCH_name = "ORNcc"; goto MATCH_label_a4; break;
              case 23: 
                MATCH_name = "XNORcc"; goto MATCH_label_a4; break;
              case 24: 
                MATCH_name = "ADDXcc"; goto MATCH_label_a4; break;
              case 26: 
                MATCH_name = "UMULcc"; goto MATCH_label_a4; break;
              case 27: 
                MATCH_name = "SMULcc"; goto MATCH_label_a4; break;
              case 28: 
                MATCH_name = "SUBXcc"; goto MATCH_label_a4; break;
              case 30: 
                MATCH_name = "UDIVcc"; goto MATCH_label_a4; break;
              case 31: 
                MATCH_name = "SDIVcc"; goto MATCH_label_a4; break;
              case 32: 
                MATCH_name = "TADDcc"; goto MATCH_label_a4; break;
              case 33: 
                MATCH_name = "TSUBcc"; goto MATCH_label_a4; break;
              case 34: 
                MATCH_name = "TADDccTV"; goto MATCH_label_a4; break;
              case 35: 
                MATCH_name = "TSUBccTV"; goto MATCH_label_a4; break;
              case 36: 
                MATCH_name = "MULScc"; goto MATCH_label_a4; break;
              case 37: 
                MATCH_name = "SLL"; goto MATCH_label_a4; break;
              case 38: 
                MATCH_name = "SRL"; goto MATCH_label_a4; break;
              case 39: 
                MATCH_name = "SRA"; goto MATCH_label_a4; break;
              case 40: 
                if ((MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0) { 
                  MATCH_name = MATCH_name_rs1_43[(MATCH_w_32_0 >> 14 & 0x1f) 
                        /* rs1 at 0 */]; 
                  { 
                    char *name = MATCH_name;
                    unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                    nextPC = 4 + MATCH_p; 
                    
                    #line 126 "machine/sparc/decoder_low.m"
                     

                    		RTs = instantiate(pc,  name, DIS_RD);

                    

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 41: 
                MATCH_name = MATCH_name_op3_44[(MATCH_w_32_0 >> 19 & 0x3f) 
                      /* op3 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 129 "machine/sparc/decoder_low.m"
                   

                  		RTs = instantiate(pc,  name, DIS_RD);

                  

                  
                  
                  
                }
                
                break;
              case 42: 
                MATCH_name = MATCH_name_op3_44[(MATCH_w_32_0 >> 19 & 0x3f) 
                      /* op3 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 132 "machine/sparc/decoder_low.m"
                   

                  		RTs = instantiate(pc,  name, DIS_RD);

                  

                  
                  
                  
                }
                
                break;
              case 43: 
                MATCH_name = MATCH_name_op3_44[(MATCH_w_32_0 >> 19 & 0x3f) 
                      /* op3 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 135 "machine/sparc/decoder_low.m"
                   

                  		RTs = instantiate(pc,  name, DIS_RD);

                  

                  
                  
                  
                }
                
                break;
              case 48: 
                if (1 <= (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ && 
                  (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ < 32) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else { 
                  MATCH_name = "WRY"; 
                  { 
                    char *name = MATCH_name;
                    unsigned roi = addressToPC(MATCH_p);
                    unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                    nextPC = 4 + MATCH_p; 
                    
                    #line 138 "machine/sparc/decoder_low.m"
                     

                    		RTs = instantiate(pc,  name, DIS_RS1, DIS_ROI);

                    

                    
                    
                    
                  }
                  
                } /*opt-block*/
                
                break;
              case 49: 
                MATCH_name = "WRPSR"; 
                { 
                  char *name = MATCH_name;
                  unsigned roi = addressToPC(MATCH_p);
                  unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 141 "machine/sparc/decoder_low.m"
                   

                  		RTs = instantiate(pc,  name, DIS_RS1, DIS_ROI);

                  

                  
                  
                  
                }
                
                break;
              case 50: 
                MATCH_name = "WRWIM"; 
                { 
                  char *name = MATCH_name;
                  unsigned roi = addressToPC(MATCH_p);
                  unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 144 "machine/sparc/decoder_low.m"
                   

                  		RTs = instantiate(pc,  name, DIS_RS1, DIS_ROI);

                  

                  
                  
                  
                }
                
                break;
              case 51: 
                MATCH_name = "WRTBR"; 
                { 
                  char *name = MATCH_name;
                  unsigned roi = addressToPC(MATCH_p);
                  unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 147 "machine/sparc/decoder_low.m"
                   

                  		RTs = instantiate(pc,  name, DIS_RS1, DIS_ROI);

                  

                  
                  
                  
                }
                
                break;
              case 52: 
                if (80 <= (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ && 
                  (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ < 196 || 
                  212 <= (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ && 
                  (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ < 512) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else 
                  switch((MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */) {
                    case 0: case 2: case 3: case 4: case 6: case 7: case 8: 
                    case 10: case 11: case 12: case 13: case 14: case 15: 
                    case 16: case 17: case 18: case 19: case 20: case 21: 
                    case 22: case 23: case 24: case 25: case 26: case 27: 
                    case 28: case 29: case 30: case 31: case 32: case 33: 
                    case 34: case 35: case 36: case 37: case 38: case 39: 
                    case 40: case 44: case 45: case 46: case 47: case 48: 
                    case 49: case 50: case 51: case 52: case 53: case 54: 
                    case 55: case 56: case 57: case 58: case 59: case 60: 
                    case 61: case 62: case 63: case 64: case 68: case 72: 
                    case 76: case 197: case 202: case 207: case 208: 
                      goto MATCH_label_a0; break;
                    case 1: case 5: case 9: case 41: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 169 "machine/sparc/decoder_low.m"
                         

                        		RTs = instantiate(pc,  name, DIS_FS2S, DIS_FDS);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 42: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 220 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2D, DIS_FDD);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 43: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 223 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2Q, DIS_FDQ);

                        

                        

                        
                        
                        
                      }
                      
                      break;
                    case 65: case 69: case 73: case 77: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs1s = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1s at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 172 "machine/sparc/decoder_low.m"
                         

                        		RTs = instantiate(pc,  name, DIS_FS1S, DIS_FS2S, DIS_FDS);

                         

                        
                        
                        
                      }
                      
                      break;
                    case 66: case 70: case 74: case 78: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                        unsigned fs1d = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1d at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 175 "machine/sparc/decoder_low.m"
                         

                        		RTs = instantiate(pc,  name, DIS_FS1D, DIS_FS2D, DIS_FDD);

                         

                        
                        
                        
                      }
                      
                      break;
                    case 67: case 71: case 75: case 79: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                        unsigned fs1q = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1q at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 178 "machine/sparc/decoder_low.m"
                         

                        		RTs = instantiate(pc,  name, DIS_FS1Q, DIS_FS2Q, DIS_FDQ);

                         

                        
                        
                        
                      }
                      
                      break;
                    case 196: case 209: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 190 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2S, DIS_FDS);

                        

                            // Note: itod and dtoi have different sized registers

                        
                        
                        
                      }
                      
                      break;
                    case 198: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 206 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2D, DIS_FDS);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 199: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 211 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2Q, DIS_FDS);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 200: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 193 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2S, DIS_FDD);

                        
                        
                        
                      }
                      
                      break;
                    case 201: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 203 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2S, DIS_FDD);

                        
                        
                        
                      }
                      
                      break;
                    case 203: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 216 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2Q, DIS_FDD);

                        

                        

                        
                        
                        
                      }
                      
                      break;
                    case 204: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 198 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2S, DIS_FDQ);

                        
                        
                        
                      }
                      
                      break;
                    case 205: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 208 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2S, DIS_FDQ);

                        
                        
                        
                      }
                      
                      break;
                    case 206: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 213 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2D, DIS_FDQ);

                        
                        
                        
                      }
                      
                      break;
                    case 210: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 196 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2D, DIS_FDS);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 211: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 201 "machine/sparc/decoder_low.m"
                        

                                RTs = instantiate(pc, name, DIS_FS2Q, DIS_FDS);

                        

                        
                        
                        
                      }
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 5 & 0x1ff) -- opf at 0 --*/ 
                break;
              case 53: 
                if (0 <= (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ && 
                  (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ < 81 || 
                  88 <= (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ && 
                  (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ < 512) 
                  goto MATCH_label_a0;  /*opt-block+*/
                else 
                  switch((MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */) {
                    case 84: 
                      goto MATCH_label_a0; break;
                    case 81: case 85: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fs1s = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1s at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 181 "machine/sparc/decoder_low.m"
                         

                        		RTs = instantiate(pc,  name, DIS_FS1S, DIS_FS2S);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 82: case 86: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fs1d = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1d at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 184 "machine/sparc/decoder_low.m"
                         

                        		RTs = instantiate(pc,  name, DIS_FS1D, DIS_FS2D);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 83: case 87: 
                      MATCH_name = 
                        MATCH_name_opf_49[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fs1q = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1q at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
                        #line 187 "machine/sparc/decoder_low.m"
                         

                        		RTs = instantiate(pc,  name, DIS_FS1Q, DIS_FS2Q);

                        

                        
                        
                        
                      }
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 5 & 0x1ff) -- opf at 0 --*/ 
                break;
              case 56: 
                MATCH_name = "JMPL"; 
                { 
                  char *name = MATCH_name;
                  unsigned addr = addressToPC(MATCH_p);
                  unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 227 "machine/sparc/decoder_low.m"
                   

                  		result.type = DD;

                  		RTs = instantiate(pc,  name, DIS_ADDR, DIS_RD);

                  

                  
                  
                  
                }
                
                break;
              case 57: 
                MATCH_name = "RETT"; 
                { 
                  char *name = MATCH_name;
                  unsigned addr = addressToPC(MATCH_p);
                  nextPC = 4 + MATCH_p; 
                  
                  #line 231 "machine/sparc/decoder_low.m"
                   

                          unused(addr);

                  		RTs = instantiate(pc,  name);

                  

                  
                  
                  
                }
                
                break;
              case 58: 
                MATCH_name = MATCH_name_cond_52[(MATCH_w_32_0 >> 25 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned addr = addressToPC(MATCH_p);
                  nextPC = 4 + MATCH_p; 
                  
                  #line 235 "machine/sparc/decoder_low.m"
                   

                  		RTs = instantiate(pc,  name, DIS_ADDR);

                  

                  
                  
                  
                }
                
                break;
              case 60: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_54[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a4; 
                  
                } /*opt-block*/
                else 
                  if ((MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0) 
                    if ((MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0) 
                      if ((MATCH_w_32_0 & 0x1f) /* rs2 at 0 */ == 0) { 
                        MATCH_name = "SAVE"; 
                        { 
                          char *name = MATCH_name;
                          nextPC = 4 + MATCH_p; 
                          
                          #line 72 "machine/sparc/decoder_low.m"
                          

                          		RTs = instantiate(pc,  name, dis_Reg("%g0"), dis_Reg("%g0"),

                          			dis_Reg("%g0"));

                          

                          
                          
                          
                        }
                        
                      } /*opt-block*/
                      else { 
                        MATCH_name = "SAVE"; 
                        goto MATCH_label_a4; 
                        
                      } /*opt-block*/ /*opt-block+*/
                    else { 
                      MATCH_name = 
                        MATCH_name_rs1_43[(MATCH_w_32_0 >> 14 & 0x1f) 
                            /* rs1 at 0 */]; 
                      goto MATCH_label_a4; 
                      
                    } /*opt-block*/ 
                  else { 
                    MATCH_name = MATCH_name_rd_2[(MATCH_w_32_0 >> 25 & 0x1f) 
                          /* rd at 0 */]; 
                    goto MATCH_label_a4; 
                    
                  } /*opt-block*/ 
                break;
              case 61: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_58[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a4; 
                  
                } /*opt-block*/
                else 
                  if ((MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0 && 
                    (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0 && 
                    (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */ == 0) { 
                    MATCH_name = "RESTORE"; 
                    { 
                      char *name = MATCH_name;
                      nextPC = 4 + MATCH_p; 
                      
                      #line 68 "machine/sparc/decoder_low.m"
                      

                      		RTs = instantiate(pc,  name, dis_Reg("%g0"), dis_Reg("%g0"),

                      			dis_Reg("%g0"));

                      

                      
                      
                      
                    }
                    
                  } /*opt-block*/
                  else { 
                    MATCH_name = "RESTORE"; 
                    goto MATCH_label_a4; 
                    
                  } /*opt-block*/ /*opt-block+*/
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 19 & 0x3f) -- op3 at 0 --*/ 
          break;
        case 3: 
          
            switch((MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */) {
              case 0: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_59[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_54[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                
                break;
              case 1: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_60[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_58[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                
                break;
              case 2: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_61[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_59[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                
                break;
              case 3: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_62[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_60[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                
                break;
              case 4: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_63[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a6; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_61[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a6; 
                  
                } /*opt-block*/
                
                break;
              case 5: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_64[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a6; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_62[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a6; 
                  
                } /*opt-block*/
                
                break;
              case 6: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_65[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a6; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_63[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a6; 
                  
                } /*opt-block*/
                
                break;
              case 7: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_66[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a6; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_64[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a6; 
                  
                } /*opt-block*/
                
                break;
              case 8: case 11: case 12: case 14: case 24: case 27: case 28: 
              case 30: case 34: case 40: case 41: case 42: case 43: case 44: 
              case 45: case 46: case 47: case 50: case 56: case 57: case 58: 
              case 59: case 60: case 61: case 62: case 63: 
                goto MATCH_label_a0; break;
              case 9: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_67[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_65[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                
                break;
              case 10: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_68[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_66[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                
                break;
              case 13: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_69[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_67[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                
                break;
              case 15: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_70[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_68[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                
                break;
              case 16: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_69[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 17: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_70[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 18: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_71[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 19: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_72[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 20: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_73[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 21: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_74[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 22: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_75[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 23: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_76[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 25: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_77[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 26: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_78[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 29: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_79[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 31: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_80[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 32: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_81[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a9; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_71[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a9; 
                  
                } /*opt-block*/
                
                break;
              case 33: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_82[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a10; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_72[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a10; 
                  
                } /*opt-block*/
                
                break;
              case 35: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_83[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a11; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_73[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a11; 
                  
                } /*opt-block*/
                
                break;
              case 36: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_84[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a12; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_74[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a12; 
                  
                } /*opt-block*/
                
                break;
              case 37: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_75[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a13; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STFSR"; 
                  goto MATCH_label_a13; 
                  
                } /*opt-block*/
                
                break;
              case 38: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_76[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a14; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STDFQ"; 
                  goto MATCH_label_a14; 
                  
                } /*opt-block*/
                
                break;
              case 39: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_77[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a15; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STDF"; 
                  goto MATCH_label_a15; 
                  
                } /*opt-block*/
                
                break;
              case 48: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_78[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a16; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "LDC"; 
                  goto MATCH_label_a16; 
                  
                } /*opt-block*/
                
                break;
              case 49: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_79[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a17; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "LDCSR"; 
                  goto MATCH_label_a17; 
                  
                } /*opt-block*/
                
                break;
              case 51: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_80[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a16; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "LDDC"; 
                  goto MATCH_label_a16; 
                  
                } /*opt-block*/
                
                break;
              case 52: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_81[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a18; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STC"; 
                  goto MATCH_label_a18; 
                  
                } /*opt-block*/
                
                break;
              case 53: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_82[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a19; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STCSR"; 
                  goto MATCH_label_a19; 
                  
                } /*opt-block*/
                
                break;
              case 54: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_83[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a20; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STDCQ"; 
                  goto MATCH_label_a20; 
                  
                } /*opt-block*/
                
                break;
              case 55: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_84[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a18; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STDC"; 
                  goto MATCH_label_a18; 
                  
                } /*opt-block*/
                
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 19 & 0x3f) -- op3 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 30 & 0x3) -- op at 0 --*/ 
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    { 
      unsigned n = MATCH_w_32_0 /* inst at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 243 "machine/sparc/decoder_low.m"
       

              // What does this mean?

              unused(n);

              result.valid = false;

      		RTs = NULL;

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a1: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned tgt = 
        4 * sign_extend((MATCH_w_32_0 & 0x3fffff) /* disp22 at 0 */, 22) + 
        addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 161 "machine/sparc/decoder_low.m"
       

      

                    RTs = instantiate(pc,  name, dis_Num((int)((tgt-hostPC))>>2) );

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a2: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned tgt = 
        4 * sign_extend((MATCH_w_32_0 & 0x3fffff) /* disp22 at 0 */, 22) + 
        addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 153 "machine/sparc/decoder_low.m"
       

      /*

      		char shortname[8];

      		strcpy(shortname, name);

      		shortname[strlen(name)-2] = '\0';		// Remove the ",a"

      */

                    RTs = instantiate(pc,  name, dis_Num((tgt-hostPC)>>2) );

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a3: (void)0; /*placeholder for label*/ 
    { 
      unsigned imm22 = (MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */ << 10;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 65 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  "sethi", dis_Num(imm22), DIS_RD);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a4: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      unsigned roi = addressToPC(MATCH_p);
      unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 150 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_RS1, DIS_ROI, DIS_RD);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a5: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 76 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_ADDR, DIS_RD);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a6: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 92 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_RD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a7: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned asi = (MATCH_w_32_0 >> 5 & 0xff) /* asi at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 88 "machine/sparc/decoder_low.m"
       

              unused(asi);            // Note: this could be serious!

      		RTs = instantiate(pc,  name, DIS_RD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a8: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned asi = (MATCH_w_32_0 >> 5 & 0xff) /* asi at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 104 "machine/sparc/decoder_low.m"
       

              unused(asi);            // Note: this could be serious!

      		RTs = instantiate(pc,  name, DIS_RD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a9: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fds = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 79 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_ADDR, DIS_FDS);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a10: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 108 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a11: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fdd = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 82 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_ADDR, DIS_FDD);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a12: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fds = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 95 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_FDS, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a13: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 114 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a14: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 120 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a15: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fdd = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 98 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_FDD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a16: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned cd = (MATCH_w_32_0 >> 25 & 0x1f) /* cd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 85 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_ADDR, DIS_CD);

      		

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a17: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 111 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a18: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned cd = (MATCH_w_32_0 >> 25 & 0x1f) /* cd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 101 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_CD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a19: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 117 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a20: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 123 "machine/sparc/decoder_low.m"
       

      		RTs = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 255 "machine/sparc/decoder_low.m"

	result.numBytes = (nextPC - hostPC);
	return RTs;
}



