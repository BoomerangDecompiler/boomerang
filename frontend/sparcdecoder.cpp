#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 1 "frontend/machine/sparc/decoder.m"
/*
 * Copyright (C) 1996-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       decoder.m
 * OVERVIEW:   Implementation of the SPARC specific parts of the
 *             SparcDecoder class.
 *============================================================================*/

/* $Revision$
 *
 * 26 Apr 02 - Mike: Mods for boomerang
 * 19 May 02 - Mike: Added many (int) casts: variables from toolkit are unsgnd
 * 21 May 02 - Mike: SAVE and RESTORE have full semantics now
 * 30 Oct 02 - Mike: dis_Eaddr mode indirectA had extra memof
 * 22 Nov 02 - Mike: Support 32 bit V9 branches
 * 04 Dec 02 - Mike: r[0] -> 0 automatically (rhs only)
*/

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "exp.h"
#include "proc.h"
#include "prog.h"
#include "decoder.h"
#include "sparcdecoder.h"
#include "rtl.h"
#include "BinaryFile.h"		// For SymbolByAddress()
#include "boomerang.h"

#define DIS_ROI     (dis_RegImm(roi))
#define DIS_ADDR    (dis_Eaddr(addr))
#define DIS_RD      (dis_RegLhs(rd))
#define DIS_RS1     (dis_RegRhs(rs1))
#define DIS_FS1S    (dis_RegRhs(fs1s+32))
#define DIS_FS2S    (dis_RegRhs(fs2s+32))
// Note: Sparc V9 has a second set of double precision registers that have an
// odd index. So far we only support V8
#define DIS_FDS     (dis_RegLhs((fds>>1)+64))
#define DIS_FS1D    (dis_RegRhs((fs1d>>1)+64))
#define DIS_FS2D    (dis_RegRhs((fs2d>>1)+64))
#define DIS_FDD     (dis_RegLhs((fdd>>1)+64))
#define DIS_FDQ     (dis_RegLhs((fdq>>2)+80))
#define DIS_FS1Q    (dis_RegRhs((fs1q>>2)+80))
#define DIS_FS2Q    (dis_RegRhs((fs2q>>2)+80))

/*==============================================================================
 * FUNCTION:       unused
 * OVERVIEW:       A dummy function to suppress "unused local variable" messages
 * PARAMETERS:     x: integer variable to be "used"
 * RETURNS:        Nothing
 *============================================================================*/
void SparcDecoder::unused(int x)
{}

/*==============================================================================
 * FUNCTION:       createJcond
 * OVERVIEW:       Create an RTL for a Bx instruction
 * PARAMETERS:     pc - the location counter
 *                  exps - ptr to list of Exp pointers
 *                  name - instruction name (e.g. "BNE,a")
 * RETURNS:        Pointer to newly created RTL, or NULL if invalid
 *============================================================================*/
HLJcond* SparcDecoder::createJcond(ADDRESS pc, std::list<Exp*>* exps, const char* name)
{
    HLJcond* res = new HLJcond(pc, exps);
    if (name[0] == 'F') {
        // fbranch is any of [ FBN FBNE FBLG FBUL FBL   FBUG FBG   FBU
        //                     FBA FBE  FBUE FBGE FBUGE FBLE FBULE FBO ],
        // fbranches are not the same as ibranches, so need a whole different
        // set of tests
        if (name[2] == 'U')
            name++;             // Just ignore unordered (for now)
        switch (name[2]) {
        case 'E':                           // FBE
            res->setCondType(HLJCOND_JE, true);
            break;
        case 'L':
            if (name[3] == 'G')             // FBLG
                res->setCondType(HLJCOND_JNE, true);
            else if (name[3] == 'E')        // FBLE
                res->setCondType(HLJCOND_JSLE, true);
            else                            // FBL
                res->setCondType(HLJCOND_JSL, true);
            break;
        case 'G':
            if (name[3] == 'E')             // FBGE
                res->setCondType(HLJCOND_JSGE, true);
            else                            // FBG
                res->setCondType(HLJCOND_JSG, true);
            break;
        case 'N':
            if (name[3] == 'E')             // FBNE
                res->setCondType(HLJCOND_JNE, true);
            // Else it's FBN!
            break;
        default:
            std::cerr << "unknown float branch " << name << std::endl;
            delete res;
            res = NULL;
        }
        return res;
    }   

    // ibranch is any of [ BN BE  BLE BL  BLEU BCS BNEG BVS
    //                     BA BNE BG  BGE BGU  BCC BPOS BVC ],
    switch(name[1]) {
    case 'E':
        res->setCondType(HLJCOND_JE);           // BE
        break;
    case 'L':
        if (name[2] == 'E') {
            if (name[3] == 'U')
                res->setCondType(HLJCOND_JULE); // BLEU
            else
                res->setCondType(HLJCOND_JSLE); // BLE
        }
        else
            res->setCondType(HLJCOND_JSL);      // BL
        break;
    case 'N':
        // BNE, BNEG (won't see BN)
        if (name[3] == 'G')
            res->setCondType(HLJCOND_JMI);      // BNEG
        else
            res->setCondType(HLJCOND_JNE);      // BNE
        break;
    case 'C':
        // BCC, BCS
        if (name[2] == 'C')
            res->setCondType(HLJCOND_JUGE);     // BCC
        else
            res->setCondType(HLJCOND_JUL);      // BCS
        break;
    case 'V':
        // BVC, BVS; should never see these now
        if (name[2] == 'C')
            std::cerr << "Decoded BVC instruction\n";   // BVC
        else
            std::cerr << "Decoded BVS instruction\n";   // BVS
        break;
    case 'G':   
        // BGE, BG, BGU
        if (name[2] == 'E')
            res->setCondType(HLJCOND_JSGE);     // BGE
        else if (name[2] == 'U')
            res->setCondType(HLJCOND_JUG);      // BGU
        else
            res->setCondType(HLJCOND_JSG);      // BG
        break;
    case 'P':   
		if (name[2] == 'O') {
        	res->setCondType(HLJCOND_JPOS);         // BPOS
        	break;
		}
		// Else, it's a BPXX; remove the P (for predicted) and try again
		// (recurse)
		// B P P O S ...
		// 0 1 2 3 4 ...
		char temp[8];
		temp[0] = 'B';
		strcpy(temp+1, name+2);
		delete res;
		return createJcond(pc, exps, temp);
    default:
        std::cerr << "unknown non-float branch " << name << std::endl;
    }   
    return res;
}


/*==============================================================================
 * FUNCTION:       SparcDecoder::decodeInstruction
 * OVERVIEW:       Attempt to decode the high level instruction at a given
 *                 address and return the corresponding HL type (e.g. HLCall,
 *                 HLJump etc). If no high level instruction exists at the
 *                 given address, then simply return the RTL for the low level
 *                 instruction at this address. There is an option to also
 *                 include the exps for a HL instruction.
 * PARAMETERS:     pc - the native address of the pc
 *                 delta - the difference between the above address and the
 *                   host address of the pc (i.e. the address that the pc is at
 *                   in the loaded object file)
 *                 proc - the enclosing procedure. This can be NULL for
 *                   those of us who are using this method in an interpreter
 * RETURNS:        a DecodeResult structure containing all the information
 *                   gathered during decoding
 *============================================================================*/
DecodeResult& SparcDecoder::decodeInstruction (ADDRESS pc, int delta)
{ 
    static DecodeResult result;
    ADDRESS hostPC = pc+delta;

    // Clear the result structure;
    result.reset();

    // The actual list of instantiated exps
    std::list<Exp*>* exps = NULL;

    ADDRESS nextPC;



#line 212 "frontend/machine/sparc/decoder.m"
{ 
  dword MATCH_p = 
    
#line 212 "frontend/machine/sparc/decoder.m"
    hostPC
    ;
  char *MATCH_name;
  static char *MATCH_name_cond_0[] = {
    "BPN", "BPE", "BPLE", "BPL", "BPLEU", "BPCS", "BPNEG", "BPVS", "BA", 
    "BPNE", "BPG", "BPGE", "BPGU", "BPCC", "BPPOS", "BPVC", 
  };
  static char *MATCH_name_cond_1[] = {
    "BN", "BE", "BLE", "BL", "BLEU", "BCS", "BNEG", "BVS", "BA,a", "BNE", 
    "BG", "BGE", "BGU", "BCC", "BPOS", "BVC", 
  };
  static char *MATCH_name_cond_2[] = {
    "BN,a", "BE,a", "BLE,a", "BL,a", "BLEU,a", "BCS,a", "BNEG,a", "BVS,a", 
    "FBA", "BNE,a", "BG,a", "BGE,a", "BGU,a", "BCC,a", "BPOS,a", "BVC,a", 
  };
  static char *MATCH_name_cond_4[] = {
    "FBN", "FBNE", "FBLG", "FBUL", "FBL", "FBUG", "FBG", "FBU", "FBA,a", 
    "FBE", "FBUE", "FBGE", "FBUGE", "FBLE", "FBULE", "FBO", 
  };
  static char *MATCH_name_cond_5[] = {
    "FBN,a", "FBNE,a", "FBLG,a", "FBUL,a", "FBL,a", "FBUG,a", "FBG,a", 
    "FBU,a", "CBA", "FBE,a", "FBUE,a", "FBGE,a", "FBUGE,a", "FBLE,a", 
    "FBULE,a", "FBO,a", 
  };
  static char *MATCH_name_cond_6[] = {
    "CBN", "CB123", "CB12", "CB13", "CB1", "CB23", "CB2", "CB3", "CBA,a", 
    "CB0", "CB03", "CB02", "CB023", "CB01", "CB013", "CB012", 
  };
  static char *MATCH_name_cond_7[] = {
    "CBN,a", "CB123,a", "CB12,a", "CB13,a", "CB1,a", "CB23,a", "CB2,a", 
    "CB3,a", "TA", "CB0,a", "CB03,a", "CB02,a", "CB023,a", "CB01,a", 
    "CB013,a", "CB012,a", 
  };
  static char *MATCH_name_op3_45[] = {
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "RDPSR", "RDWIM", 
    "RDTBR", 
  };
  static char *MATCH_name_opf_50[] = {
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
    "TN", "TE", "TLE", "TL", "TLEU", "TCS", "TNEG", "TVS", (char *)0, "TNE", 
    "TG", "TGE", "TGU", "TCC", "TPOS", "TVC", 
  };
  static char *MATCH_name_i_65[] = {"LDA", "LDF", };
  static char *MATCH_name_i_66[] = {"LDUBA", "LDFSR", };
  static char *MATCH_name_i_67[] = {"LDUHA", "LDDF", };
  static char *MATCH_name_i_68[] = {"LDDA", "STF", };
  static char *MATCH_name_i_69[] = {"STA", "STFSR", };
  static char *MATCH_name_i_70[] = {"STBA", "STDFQ", };
  static char *MATCH_name_i_71[] = {"STHA", "STDF", };
  static char *MATCH_name_i_72[] = {"STDA", "LDCSR", };
  static char *MATCH_name_i_73[] = {"LDSBA", "STCSR", };
  static char *MATCH_name_i_74[] = {"LDSHA", "STDCQ", };
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
                  
#line 556 "frontend/machine/sparc/decoder.m"
                   

                          unused(n);

                  		exps = NULL;

                          result.valid = false;

                  

                  
                  
                  
                }
                
                break;
              case 1: 
                if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1) 
                  goto MATCH_label_d0;  /*opt-block+*/
                else 
                  if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) { 
                    unsigned cc01 = 
                      (MATCH_w_32_0 >> 20 & 0x3) /* cc01 at 0 */;
                    unsigned tgt = 
                      4 * sign_extend(
                                  (MATCH_w_32_0 & 0x7ffff) /* disp19 at 0 */, 
                                  19) + addressToPC(MATCH_p);
                    nextPC = 4 + MATCH_p; 
                    
#line 336 "frontend/machine/sparc/decoder.m"
                    			/* Can see bpa xcc,tgt in 32 bit code */

                    		unused(cc01);				// Does not matter because is unconditional

                            HLJump* jump = 0;

                            jump = new HLJump(pc, exps);

                    

                            result.type = SD;

                            result.rtl = jump;

                            jump->setDest(tgt - delta);

                            SHOW_ASM("BPA " << hex << tgt-delta)

                    

                    
                    
                    
                  } /*opt-block*//*opt-block+*/
                  else { 
                    MATCH_name = MATCH_name_cond_0[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    { 
                      char *name = MATCH_name;
                      unsigned cc01 = 
                        (MATCH_w_32_0 >> 20 & 0x3) /* cc01 at 0 */;
                      unsigned tgt = 
                        4 * sign_extend(
                                    (MATCH_w_32_0 & 0x7ffff) 
                                          /* disp19 at 0 */, 19) + 
                        addressToPC(MATCH_p);
                      nextPC = 4 + MATCH_p; 
                      
#line 346 "frontend/machine/sparc/decoder.m"
                      

                              if (cc01 != 0) {		/* If 64 bit cc used, can't handle */

                                  result.valid = false;

                                  result.rtl = new RTL;

                                  result.numBytes = 4;

                                  return result;

                              }

                              HLJump* jump = 0;

                              if (strcmp(name,"BPN") == 0)

                                  jump = new HLJump(pc, exps);

                              if ((jump == 0) &&

                                (strcmp(name,"BPVS") == 0 || strcmp(name,"BPVC") == 0))

                                  jump = new HLJump(pc, exps);

                              if (jump == 0)

                                  jump = createJcond(pc, exps, name);

                      

                              // The class of this instruction depends on whether or not

                              // it is one of the 'unconditional' conditional branches

                              // "BPN" (or the pseudo unconditionals BPVx)

                              result.type = SCD;

                              if (strcmp(name, "BPVC") == 0)

                                  result.type = SD;

                              if ((strcmp(name,"BPN") == 0) || (strcmp(name, "BPVS") == 0))

                                  result.type = NCT;

                      

                              result.rtl = jump;

                              jump->setDest(tgt - delta);

                              SHOW_ASM(name << " " << hex << tgt-delta)

                      

                      

                      
                      
                      
                    }
                    
                  } /*opt-block*/ /*opt-block+*/
                break;
              case 2: 
                if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1) 
                  if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) { 
                    MATCH_name = MATCH_name_cond_1[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d2; 
                    
                  } /*opt-block*/
                  else { 
                    MATCH_name = MATCH_name_cond_2[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d2; 
                    
                  } /*opt-block*/ /*opt-block+*/
                else 
                  if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) { 
                    MATCH_name = MATCH_name_cond_0[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d1; 
                    
                  } /*opt-block*/
                  else { 
                    MATCH_name = MATCH_name_cond_1[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d1; 
                    
                  } /*opt-block*/ /*opt-block+*/
                break;
              case 3: case 5: 
                goto MATCH_label_d0; break;
              case 4: 
                if ((MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */ == 0 && 
                  (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0) { 
                  MATCH_name = "NOP"; 
                  { 
                    char *name = MATCH_name;
                    nextPC = 4 + MATCH_p; 
                    
#line 410 "frontend/machine/sparc/decoder.m"
                    

                    		result.type = NOP;

                    		exps = instantiate(pc,  name);

                    

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d3;  /*opt-block+*/
                
                break;
              case 6: 
                if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1) 
                  if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) { 
                    MATCH_name = MATCH_name_cond_4[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d2; 
                    
                  } /*opt-block*/
                  else { 
                    MATCH_name = MATCH_name_cond_5[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d2; 
                    
                  } /*opt-block*/ /*opt-block+*/
                else 
                  if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) { 
                    MATCH_name = MATCH_name_cond_2[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d1; 
                    
                  } /*opt-block*/
                  else { 
                    MATCH_name = MATCH_name_cond_4[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d1; 
                    
                  } /*opt-block*/ /*opt-block+*/
                break;
              case 7: 
                if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1) 
                  if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) { 
                    MATCH_name = MATCH_name_cond_6[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d2; 
                    
                  } /*opt-block*/
                  else { 
                    MATCH_name = MATCH_name_cond_7[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d2; 
                    
                  } /*opt-block*/ /*opt-block+*/
                else 
                  if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) { 
                    MATCH_name = MATCH_name_cond_5[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d1; 
                    
                  } /*opt-block*/
                  else { 
                    MATCH_name = MATCH_name_cond_6[(MATCH_w_32_0 >> 25 & 0xf) 
                          /* cond at 0 */]; 
                    goto MATCH_label_d1; 
                    
                  } /*opt-block*/ /*opt-block+*/
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 22 & 0x7) -- op2 at 0 --*/ 
          break;
        case 1: 
          { 
            unsigned addr = 
              4 * sign_extend((MATCH_w_32_0 & 0x3fffffff) /* disp30 at 0 */, 
                          30) + addressToPC(MATCH_p);
            nextPC = 4 + MATCH_p; 
            
#line 215 "frontend/machine/sparc/decoder.m"
            

                    /*

                     * A standard call 

                     */

                    HLCall* newCall = new HLCall(pc, 0, 0);

            

                    // Set the destination

                    newCall->setDest(addr - delta);

                    result.rtl = newCall;

                    result.type = SD;

                    SHOW_ASM("call__ ")

            

            
            
            
          }
          
          break;
        case 2: 
          
            switch((MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */) {
              case 0: 
                MATCH_name = "ADD"; goto MATCH_label_d4; break;
              case 1: 
                MATCH_name = "AND"; goto MATCH_label_d4; break;
              case 2: 
                MATCH_name = "OR"; goto MATCH_label_d4; break;
              case 3: 
                MATCH_name = "XOR"; goto MATCH_label_d4; break;
              case 4: 
                MATCH_name = "SUB"; goto MATCH_label_d4; break;
              case 5: 
                MATCH_name = "ANDN"; goto MATCH_label_d4; break;
              case 6: 
                MATCH_name = "ORN"; goto MATCH_label_d4; break;
              case 7: 
                MATCH_name = "XNOR"; goto MATCH_label_d4; break;
              case 8: 
                MATCH_name = "ADDX"; goto MATCH_label_d4; break;
              case 9: case 13: case 25: case 29: case 44: case 45: case 46: 
              case 47: case 54: case 55: case 59: case 62: case 63: 
                goto MATCH_label_d0; break;
              case 10: 
                MATCH_name = "UMUL"; goto MATCH_label_d4; break;
              case 11: 
                MATCH_name = "SMUL"; goto MATCH_label_d4; break;
              case 12: 
                MATCH_name = "SUBX"; goto MATCH_label_d4; break;
              case 14: 
                MATCH_name = "UDIV"; goto MATCH_label_d4; break;
              case 15: 
                MATCH_name = "SDIV"; goto MATCH_label_d4; break;
              case 16: 
                MATCH_name = "ADDcc"; goto MATCH_label_d4; break;
              case 17: 
                MATCH_name = "ANDcc"; goto MATCH_label_d4; break;
              case 18: 
                MATCH_name = "ORcc"; goto MATCH_label_d4; break;
              case 19: 
                MATCH_name = "XORcc"; goto MATCH_label_d4; break;
              case 20: 
                MATCH_name = "SUBcc"; goto MATCH_label_d4; break;
              case 21: 
                MATCH_name = "ANDNcc"; goto MATCH_label_d4; break;
              case 22: 
                MATCH_name = "ORNcc"; goto MATCH_label_d4; break;
              case 23: 
                MATCH_name = "XNORcc"; goto MATCH_label_d4; break;
              case 24: 
                MATCH_name = "ADDXcc"; goto MATCH_label_d4; break;
              case 26: 
                MATCH_name = "UMULcc"; goto MATCH_label_d4; break;
              case 27: 
                MATCH_name = "SMULcc"; goto MATCH_label_d4; break;
              case 28: 
                MATCH_name = "SUBXcc"; goto MATCH_label_d4; break;
              case 30: 
                MATCH_name = "UDIVcc"; goto MATCH_label_d4; break;
              case 31: 
                MATCH_name = "SDIVcc"; goto MATCH_label_d4; break;
              case 32: 
                MATCH_name = "TADDcc"; goto MATCH_label_d4; break;
              case 33: 
                MATCH_name = "TSUBcc"; goto MATCH_label_d4; break;
              case 34: 
                MATCH_name = "TADDccTV"; goto MATCH_label_d4; break;
              case 35: 
                MATCH_name = "TSUBccTV"; goto MATCH_label_d4; break;
              case 36: 
                MATCH_name = "MULScc"; goto MATCH_label_d4; break;
              case 37: 
                MATCH_name = "SLL"; goto MATCH_label_d4; break;
              case 38: 
                MATCH_name = "SRL"; goto MATCH_label_d4; break;
              case 39: 
                MATCH_name = "SRA"; goto MATCH_label_d4; break;
              case 40: 
                if ((MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0) { 
                  MATCH_name = "RDY"; 
                  { 
                    char *name = MATCH_name;
                    unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                    nextPC = 4 + MATCH_p; 
                    
#line 461 "frontend/machine/sparc/decoder.m"
                     

                    		exps = instantiate(pc,  name, DIS_RD);

                    

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 41: 
                MATCH_name = MATCH_name_op3_45[(MATCH_w_32_0 >> 19 & 0x3f) 
                      /* op3 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
#line 464 "frontend/machine/sparc/decoder.m"
                   

                  		exps = instantiate(pc,  name, DIS_RD);

                  

                  
                  
                  
                }
                
                break;
              case 42: 
                MATCH_name = MATCH_name_op3_45[(MATCH_w_32_0 >> 19 & 0x3f) 
                      /* op3 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
#line 467 "frontend/machine/sparc/decoder.m"
                   

                  		exps = instantiate(pc,  name, DIS_RD);

                  

                  
                  
                  
                }
                
                break;
              case 43: 
                MATCH_name = MATCH_name_op3_45[(MATCH_w_32_0 >> 19 & 0x3f) 
                      /* op3 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
#line 470 "frontend/machine/sparc/decoder.m"
                   

                  		exps = instantiate(pc,  name, DIS_RD);

                  

                  
                  
                  
                }
                
                break;
              case 48: 
                if (1 <= (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ && 
                  (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ < 32) 
                  goto MATCH_label_d0;  /*opt-block+*/
                else { 
                  MATCH_name = "WRY"; 
                  { 
                    char *name = MATCH_name;
                    unsigned roi = addressToPC(MATCH_p);
                    unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                    nextPC = 4 + MATCH_p; 
                    
#line 473 "frontend/machine/sparc/decoder.m"
                     

                    		exps = instantiate(pc,  name, DIS_RS1, DIS_ROI);

                    

                    
                    
                    
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
                  
#line 476 "frontend/machine/sparc/decoder.m"
                   

                  		exps = instantiate(pc,  name, DIS_RS1, DIS_ROI);

                  

                  
                  
                  
                }
                
                break;
              case 50: 
                MATCH_name = "WRWIM"; 
                { 
                  char *name = MATCH_name;
                  unsigned roi = addressToPC(MATCH_p);
                  unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
#line 479 "frontend/machine/sparc/decoder.m"
                   

                  		exps = instantiate(pc,  name, DIS_RS1, DIS_ROI);

                  

                  
                  
                  
                }
                
                break;
              case 51: 
                MATCH_name = "WRTBR"; 
                { 
                  char *name = MATCH_name;
                  unsigned roi = addressToPC(MATCH_p);
                  unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
#line 482 "frontend/machine/sparc/decoder.m"
                   

                  		exps = instantiate(pc,  name, DIS_RS1, DIS_ROI);

                  

                  
                  
                  
                }
                
                break;
              case 52: 
                if (80 <= (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ && 
                  (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ < 196 || 
                  212 <= (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ && 
                  (MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ < 512) 
                  goto MATCH_label_d0;  /*opt-block+*/
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
                      goto MATCH_label_d0; break;
                    case 1: case 5: case 9: case 41: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 488 "frontend/machine/sparc/decoder.m"
                         

                        		exps = instantiate(pc,  name, DIS_FS2S, DIS_FDS);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 42: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 539 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2D, DIS_FDD);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 43: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 542 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2Q, DIS_FDQ);

                        

                        

                        	// In V9, the privileged RETT becomes user-mode RETURN

                        	// It has the semantics of "ret restore" without the add part of the restore

                        
                        
                        
                      }
                      
                      break;
                    case 65: case 69: case 73: case 77: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs1s = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1s at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 491 "frontend/machine/sparc/decoder.m"
                         

                        		exps = instantiate(pc,  name, DIS_FS1S, DIS_FS2S, DIS_FDS);

                         

                        
                        
                        
                      }
                      
                      break;
                    case 66: case 70: case 74: case 78: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                        unsigned fs1d = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1d at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 494 "frontend/machine/sparc/decoder.m"
                         

                        		exps = instantiate(pc,  name, DIS_FS1D, DIS_FS2D, DIS_FDD);

                         

                        
                        
                        
                      }
                      
                      break;
                    case 67: case 71: case 75: case 79: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                        unsigned fs1q = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1q at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 497 "frontend/machine/sparc/decoder.m"
                         

                        		exps = instantiate(pc,  name, DIS_FS1Q, DIS_FS2Q, DIS_FDQ);

                         

                        
                        
                        
                      }
                      
                      break;
                    case 196: case 209: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 509 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2S, DIS_FDS);

                        

                            // Note: itod and dtoi have different sized registers

                        
                        
                        
                      }
                      
                      break;
                    case 198: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 525 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2D, DIS_FDS);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 199: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 530 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2Q, DIS_FDS);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 200: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 512 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2S, DIS_FDD);

                        
                        
                        
                      }
                      
                      break;
                    case 201: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 522 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2S, DIS_FDD);

                        
                        
                        
                      }
                      
                      break;
                    case 203: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdd = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 535 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2Q, DIS_FDD);

                        

                        

                        
                        
                        
                      }
                      
                      break;
                    case 204: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 517 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2S, DIS_FDQ);

                        
                        
                        
                      }
                      
                      break;
                    case 205: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 527 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2S, DIS_FDQ);

                        
                        
                        
                      }
                      
                      break;
                    case 206: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fdq = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 532 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2D, DIS_FDQ);

                        
                        
                        
                      }
                      
                      break;
                    case 210: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 515 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2D, DIS_FDS);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 211: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fds = 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 520 "frontend/machine/sparc/decoder.m"
                        

                                exps = instantiate(pc, name, DIS_FS2Q, DIS_FDS);

                        

                        
                        
                        
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
                  goto MATCH_label_d0;  /*opt-block+*/
                else 
                  switch((MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */) {
                    case 84: 
                      goto MATCH_label_d0; break;
                    case 81: case 85: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fs1s = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1s at 0 */;
                        unsigned fs2s = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 500 "frontend/machine/sparc/decoder.m"
                         

                        		exps = instantiate(pc,  name, DIS_FS1S, DIS_FS2S);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 82: case 86: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fs1d = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1d at 0 */;
                        unsigned fs2d = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 503 "frontend/machine/sparc/decoder.m"
                         

                        		exps = instantiate(pc,  name, DIS_FS1D, DIS_FS2D);

                        

                        
                        
                        
                      }
                      
                      break;
                    case 83: case 87: 
                      MATCH_name = 
                        MATCH_name_opf_50[(MATCH_w_32_0 >> 5 & 0x1ff) 
                            /* opf at 0 */]; 
                      { 
                        char *name = MATCH_name;
                        unsigned fs1q = 
                          (MATCH_w_32_0 >> 14 & 0x1f) /* fs1q at 0 */;
                        unsigned fs2q = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                        nextPC = 4 + MATCH_p; 
                        
#line 506 "frontend/machine/sparc/decoder.m"
                         

                        		exps = instantiate(pc,  name, DIS_FS1Q, DIS_FS2Q);

                        

                        
                        
                        
                      }
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 5 & 0x1ff) -- opf at 0 --*/ 
                break;
              case 56: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) 
                  
                    switch((MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */) {
                      case 0: 
                        if ((MATCH_w_32_0 >> 14 & 0x1f) 
                                /* rs1 at 0 */ == 31 && 
                          (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ == 8) { 
                          nextPC = 4 + MATCH_p; 
                          
#line 244 "frontend/machine/sparc/decoder.m"
                          

                                  /*

                                   * Just a ret, no restore (? not sure now)

                                   */

                                  result.rtl = new HLReturn(pc, exps);

                                  result.type = DD;

                                  SHOW_ASM("ret_")

                          

                          
                          
                          
                        } /*opt-block*//*opt-block+*/
                        else 
                          goto MATCH_label_d5;  /*opt-block+*/
                        
                        break;
                      case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
                      case 8: case 9: case 10: case 11: case 12: case 13: 
                      case 14: case 16: case 17: case 18: case 19: case 20: 
                      case 21: case 22: case 23: case 24: case 25: case 26: 
                      case 27: case 28: case 29: case 30: case 31: 
                        goto MATCH_label_d5; break;
                      case 15: 
                        goto MATCH_label_d6; break;
                      default: assert(0);
                    } /* (MATCH_w_32_0 >> 25 & 0x1f) -- rd at 0 --*/  
                else 
                  if ((MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 15) 
                    goto MATCH_label_d6;  /*opt-block+*/
                  else 
                    goto MATCH_label_d5;  /*opt-block+*/ /*opt-block+*/
                break;
              case 57: 
                MATCH_name = "RETURN"; 
                { 
                  char *name = MATCH_name;
                  unsigned addr = addressToPC(MATCH_p);
                  nextPC = 4 + MATCH_p; 
                  
#line 548 "frontend/machine/sparc/decoder.m"
                   

                  		exps = instantiate(pc, name, DIS_ADDR);

                          result.rtl = new HLReturn(pc, exps);

                          result.type = DD;

                  

                  
                  
                  
                }
                
                break;
              case 58: 
                if (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ && 
                  (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2 && 
                  (MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) { 
                  MATCH_name = MATCH_name_cond_7[(MATCH_w_32_0 >> 25 & 0xf) 
                        /* cond at 0 */]; 
                  goto MATCH_label_d7; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = MATCH_name_cond_52[(MATCH_w_32_0 >> 25 & 0xf) 
                        /* cond at 0 */]; 
                  goto MATCH_label_d7; 
                  
                } /*opt-block*/
                
                break;
              case 60: 
                { 
                  unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                  unsigned roi = addressToPC(MATCH_p);
                  unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
#line 400 "frontend/machine/sparc/decoder.m"
                  

                          // Decided to treat SAVE as an ordinary instruction

                          // That is, use the large list of effects from the SSL file, and

                          // hope that optimisation will vastly help the common cases

                          exps = instantiate(pc, "SAVE", DIS_RS1, DIS_ROI, DIS_RD);

                  

                  
                  
                  
                }
                
                break;
              case 61: 
                { 
                  unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                  unsigned roi = addressToPC(MATCH_p);
                  unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
#line 406 "frontend/machine/sparc/decoder.m"
                  

                          // Decided to treat RESTORE as an ordinary instruction

                          exps = instantiate(pc, "RESTORE", DIS_RS1, DIS_ROI, DIS_RD);

                  

                  
                  
                  
                }
                
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 19 & 0x3f) -- op3 at 0 --*/ 
          break;
        case 3: 
          
            switch((MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */) {
              case 0: 
                MATCH_name = "LD"; goto MATCH_label_d8; break;
              case 1: 
                MATCH_name = "LDUB"; goto MATCH_label_d8; break;
              case 2: 
                MATCH_name = "LDUH"; goto MATCH_label_d8; break;
              case 3: 
                MATCH_name = "LDD"; goto MATCH_label_d8; break;
              case 4: 
                MATCH_name = "ST"; goto MATCH_label_d9; break;
              case 5: 
                MATCH_name = "STB"; goto MATCH_label_d9; break;
              case 6: 
                MATCH_name = "STH"; goto MATCH_label_d9; break;
              case 7: 
                MATCH_name = "STD"; goto MATCH_label_d9; break;
              case 8: case 11: case 12: case 14: case 24: case 27: case 28: 
              case 30: case 34: case 40: case 41: case 42: case 43: case 44: 
              case 45: case 46: case 47: case 48: case 50: case 51: case 52: 
              case 55: case 56: case 57: case 58: case 59: case 60: case 61: 
              case 62: case 63: 
                goto MATCH_label_d0; break;
              case 9: 
                MATCH_name = "LDSB"; goto MATCH_label_d8; break;
              case 10: 
                MATCH_name = "LDSH"; goto MATCH_label_d8; break;
              case 13: 
                MATCH_name = "LDSTUB"; goto MATCH_label_d8; break;
              case 15: 
                MATCH_name = "SWAP."; goto MATCH_label_d8; break;
              case 16: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_65[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 17: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_66[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 18: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_67[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 19: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_68[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 20: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_69[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d11; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 21: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_70[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d11; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 22: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_71[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d11; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 23: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_72[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d11; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 25: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_73[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 26: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_74[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 29: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = "LDSTUBA"; 
                  goto MATCH_label_d10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 31: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = "SWAPA"; 
                  goto MATCH_label_d10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_d0;  /*opt-block+*/
                
                break;
              case 32: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_65[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d12; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "LDF"; 
                  goto MATCH_label_d12; 
                  
                } /*opt-block*/
                
                break;
              case 33: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_66[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d13; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "LDFSR"; 
                  goto MATCH_label_d13; 
                  
                } /*opt-block*/
                
                break;
              case 35: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_67[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d14; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "LDDF"; 
                  goto MATCH_label_d14; 
                  
                } /*opt-block*/
                
                break;
              case 36: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_68[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d15; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STF"; 
                  goto MATCH_label_d15; 
                  
                } /*opt-block*/
                
                break;
              case 37: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_69[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d16; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STFSR"; 
                  goto MATCH_label_d16; 
                  
                } /*opt-block*/
                
                break;
              case 38: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_70[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d17; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STDFQ"; 
                  goto MATCH_label_d17; 
                  
                } /*opt-block*/
                
                break;
              case 39: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_71[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d18; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STDF"; 
                  goto MATCH_label_d18; 
                  
                } /*opt-block*/
                
                break;
              case 49: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_72[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d19; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "LDCSR"; 
                  goto MATCH_label_d19; 
                  
                } /*opt-block*/
                
                break;
              case 53: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_73[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d20; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STCSR"; 
                  goto MATCH_label_d20; 
                  
                } /*opt-block*/
                
                break;
              case 54: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_74[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_d21; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STDCQ"; 
                  goto MATCH_label_d21; 
                  
                } /*opt-block*/
                
                break;
              default: assert(0);
            } /* (MATCH_w_32_0 >> 19 & 0x3f) -- op3 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_32_0 >> 30 & 0x3) -- op at 0 --*/ 
    
  }goto MATCH_finished_d; 
  
  MATCH_label_d0: (void)0; /*placeholder for label*/ 
    { 
      unsigned n = MATCH_w_32_0 /* inst at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 561 "frontend/machine/sparc/decoder.m"
       

              // What does this mean?

              unused(n);

              result.valid = false;

      		exps = NULL;

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d1: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned tgt = 
        4 * sign_extend((MATCH_w_32_0 & 0x3fffff) /* disp22 at 0 */, 22) + 
        addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
#line 298 "frontend/machine/sparc/decoder.m"
       

              /*

               * Non anulled branch

               */

              // First, check for CBxxx branches (branches that depend on

              // co-processor instructions). These are invalid, as far as

              // we are concerned

              if (name[0] == 'C') {

                  result.valid = false;

                  result.rtl = new RTL;

                  result.numBytes = 4;

                  return result;

              }

              // Instantiate a HLJump for the unconditional branches,

              // HLJconds for the rest

              // NOTE: NJMC toolkit cannot handle embedded else statements!

              HLJump* jump = 0;

              if (strcmp(name,"BA") == 0 || strcmp(name,"BN") == 0)

                  jump = new HLJump(pc, exps);

              if ((jump == 0) &&

                (strcmp(name,"BVS") == 0 || strcmp(name,"BVC") == 0))

                  jump = new HLJump(pc, exps);

              if (jump == 0)

                  jump = createJcond(pc, exps, name);

      

              // The class of this instruction depends on whether or not

              // it is one of the 'unconditional' conditional branches

              // "BA" or "BN" (or the pseudo unconditionals BVx)

              result.type = SCD;

              if ((strcmp(name,"BA") == 0) || (strcmp(name, "BVC") == 0))

                  result.type = SD;

              if ((strcmp(name,"BN") == 0) || (strcmp(name, "BVS") == 0))

                  result.type = NCT;

      

              result.rtl = jump;

              jump->setDest(tgt - delta);

              SHOW_ASM(name << " " << hex << tgt-delta)

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d2: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned tgt = 
        4 * sign_extend((MATCH_w_32_0 & 0x3fffff) /* disp22 at 0 */, 22) + 
        addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
#line 252 "frontend/machine/sparc/decoder.m"
       

              /*

               * Anulled branch

               */

      

              // First, check for CBxxx branches (branches that depend on

              // co-processor instructions). These are invalid, as far as

              // we are concerned

              if (name[0] == 'C') {

                  result.valid = false;

                  result.rtl = new RTL;

                  result.numBytes = 4;

                  return result;

              }

              // Instantiate a HLJump for the unconditional branches,

              // HLJconds for the rest.

              // NOTE: NJMC toolkit cannot handle embedded else statements!

              HLJump* jump = 0;

              if (strcmp(name,"BA,a") == 0 || strcmp(name,"BN,a") == 0)

                  jump = new HLJump(pc, exps);

              if ((jump == 0) &&

                (strcmp(name,"BVS,a") == 0 || strcmp(name,"BVC,a") == 0))

                  jump = new HLJump(pc, exps);

              if (jump == 0)

                  jump = createJcond(pc, exps, name);

      

              if (jump == NULL) {

                  result.valid = false;

                  result.rtl = new RTL;

                  result.numBytes = 4;

                  return result;

              }

      

              // The class of this instruction depends on whether or not

              // it is one of the 'unconditional' conditional branches

              // "BA,A" or "BN,A"

              result.type = SCDAN;

              if ((strcmp(name,"BA,a") == 0) || (strcmp(name, "BVC,a") == 0))

                  result.type = SU;

              if ((strcmp(name,"BN,a") == 0) || (strcmp(name, "BVS,a") == 0))

                  result.type = SKIP;

      

              result.rtl = jump;

              jump->setDest(tgt - delta);

              SHOW_ASM(name << " " << hex << tgt-delta)

              

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d3: (void)0; /*placeholder for label*/ 
    { 
      unsigned imm22 = (MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */ << 10;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 414 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  "sethi", dis_Num(imm22), DIS_RD);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d4: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      unsigned roi = addressToPC(MATCH_p);
      unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 485 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_RS1, DIS_ROI, DIS_RD);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d5: (void)0; /*placeholder for label*/ 
    { 
      unsigned addr = addressToPC(MATCH_p);
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 376 "frontend/machine/sparc/decoder.m"
      

              /*

               * JMPL, with rd != %o7, i.e. register jump

      		 * Note: if rd==%o7, then would be handled with the call_ arm

               */

              HLNwayJump* jump = new HLNwayJump(pc, exps);

              // Record the fact that it is a computed jump

              jump->setIsComputed();

              result.rtl = jump;

              result.type = DD;

              jump->setDest(dis_Eaddr(addr));

              unused(rd);

              SHOW_ASM("JMPL ")

      #if DEBUG_DECODER

              jump->getDest()->print();

      #endif

      

      

          //  //  //  //  //  //  //  //

          //                          //

          //   Ordinary instructions  //

          //                          //

          //  //  //  //  //  //  //  //

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d6: (void)0; /*placeholder for label*/ 
    { 
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
#line 227 "frontend/machine/sparc/decoder.m"
      

              /*

               * A JMPL with rd == %o7, i.e. a register call

               */

              HLCall* newCall = new HLCall(pc, 0, 0);

      

              // Record the fact that this is a computed call

              newCall->setIsComputed();

      

              // Set the destination expression

              newCall->setDest(dis_Eaddr(addr));

              result.rtl = newCall;

              result.type = DD;

      

              SHOW_ASM("call_ ")

      

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d7: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
#line 553 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d8: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 417 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_ADDR, DIS_RD);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d9: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 430 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_RD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d10: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned asi = (MATCH_w_32_0 >> 5 & 0xff) /* asi at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 426 "frontend/machine/sparc/decoder.m"
       

              unused(asi);            // Note: this could be serious!

      		exps = instantiate(pc,  name, DIS_RD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d11: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned asi = (MATCH_w_32_0 >> 5 & 0xff) /* asi at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 439 "frontend/machine/sparc/decoder.m"
       

              unused(asi);            // Note: this could be serious!

      		exps = instantiate(pc,  name, DIS_RD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d12: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fds = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 420 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_ADDR, DIS_FDS);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d13: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
#line 443 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d14: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fdd = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 423 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_ADDR, DIS_FDD);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d15: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fds = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 433 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_FDS, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d16: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
#line 449 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d17: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
#line 455 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d18: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fdd = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
      nextPC = 4 + MATCH_p; 
      
#line 436 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_FDD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d19: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
#line 446 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d20: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
#line 452 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d21: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
#line 458 "frontend/machine/sparc/decoder.m"
       

      		exps = instantiate(pc,  name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_finished_d: (void)0; /*placeholder for label*/
  
}

#line 572 "frontend/machine/sparc/decoder.m"

    result.numBytes = nextPC - hostPC;
    if (result.valid && result.rtl == 0)    // Don't override higher level res
        result.rtl = new RTL(pc, exps);

    return result;
}


/***********************************************************************
 * These are functions used to decode instruction operands into
 * expressions (Exp*s).
 **********************************************************************/

/*==============================================================================
 * FUNCTION:        SparcDecoder::dis_RegLhs
 * OVERVIEW:        Decode the register on the LHS
 * PARAMETERS:      r - register (0-31)
 * RETURNS:         the expression representing the register
 *============================================================================*/
Exp* SparcDecoder::dis_RegLhs(unsigned r)
{
	return new  Unary(opRegOf, new Const((int) r));
}

/*==============================================================================
 * FUNCTION:        SparcDecoder::dis_RegRhs
 * OVERVIEW:        Decode the register on the RHS
 * NOTE:            Replaces r[0] with const 0
 * NOTE:			Not used by DIS_RD since don't want 0 on LHS
 * PARAMETERS:      r - register (0-31)
 * RETURNS:         the expression representing the register
 *============================================================================*/
Exp* SparcDecoder::dis_RegRhs(unsigned r)
{
	if (r == 0)
		return new Const(0);
	return new  Unary(opRegOf, new Const((int) r));
}

/*==============================================================================
 * FUNCTION:        SparcDecoder::dis_RegImm
 * OVERVIEW:        Decode the register or immediate at the given
 *                  address.
 * NOTE:            Used via macro DIS_ROI
 * PARAMETERS:      pc - an address in the instruction stream
 * RETURNS:         the register or immediate at the given address
 *============================================================================*/
Exp* SparcDecoder::dis_RegImm(unsigned pc)
{



#line 622 "frontend/machine/sparc/decoder.m"
{ 
  dword MATCH_p = 
    
#line 622 "frontend/machine/sparc/decoder.m"
    pc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
      int /* [~4096..4095] */ i = 
        sign_extend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
      
#line 624 "frontend/machine/sparc/decoder.m"
      

              Exp* expr = new Const(i);

              return expr;

      
      
      
    } /*opt-block*//*opt-block+*/
    else { 
      unsigned rs2 = (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */;
      
#line 626 "frontend/machine/sparc/decoder.m"
      

      		return dis_RegRhs(rs2);

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_c; 
  
  MATCH_finished_c: (void)0; /*placeholder for label*/
  
}

#line 630 "frontend/machine/sparc/decoder.m"
}

/*==============================================================================
 * FUNCTION:        SparcDecoder::dis_Eaddr
 * OVERVIEW:        Converts a dynamic address to a Exp* expression.
 *                  E.g. %o7 --> r[ 15 ]
 * PARAMETERS:      pc - the instruction stream address of the dynamic
 *                    address
 *                  ignore - redundant parameter on SPARC
 * RETURNS:         the Exp* representation of the given address
 *============================================================================*/
Exp* SparcDecoder::dis_Eaddr(ADDRESS pc, int ignore /* = 0 */)
{
    Exp* expr;



#line 644 "frontend/machine/sparc/decoder.m"
{ 
  dword MATCH_p = 
    
#line 644 "frontend/machine/sparc/decoder.m"
    pc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) 
      if ((MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0) { 
        int /* [~4096..4095] */ i = 
          sign_extend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
        
#line 651 "frontend/machine/sparc/decoder.m"
        

                expr = new Const((int)i);

        
        
        
      } /*opt-block*//*opt-block+*/
      else { 
        int /* [~4096..4095] */ i = 
          sign_extend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
        unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
        
#line 654 "frontend/machine/sparc/decoder.m"
        

                expr = new Binary(opPlus,

                    new Unary(opRegOf, new Const((int)rs1)),

                    new Const((int)i));

        
        
        
      } /*opt-block*//*opt-block+*/ /*opt-block+*/
    else 
      if ((MATCH_w_32_0 & 0x1f) /* rs2 at 0 */ == 0) { 
        unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
        
#line 645 "frontend/machine/sparc/decoder.m"
        

                expr = new Unary(opRegOf, new Const((int)rs1));

        
        
        
      } /*opt-block*//*opt-block+*/
      else { 
        unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
        unsigned rs2 = (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */;
        
#line 648 "frontend/machine/sparc/decoder.m"
        

                expr = new Binary(opPlus,

                    new Unary(opRegOf, new Const((int)rs1)),

                    new Unary(opRegOf, new Const((int)rs2)));

        
        
        
      } /*opt-block*//*opt-block+*/ /*opt-block+*/
    
  }goto MATCH_finished_b; 
  
  MATCH_finished_b: (void)0; /*placeholder for label*/
  
}

#line 659 "frontend/machine/sparc/decoder.m"

    return expr;
}

/*==============================================================================
 * FUNCTION:      isFuncPrologue()
 * OVERVIEW:      Check to see if the instructions at the given offset match
 *                  any callee prologue, i.e. does it look like this offset
 *                  is a pointer to a function?
 * PARAMETERS:    hostPC - pointer to the code in question (host address)
 * RETURNS:       True if a match found
 *============================================================================*/
bool SparcDecoder::isFuncPrologue(ADDRESS hostPC)
{
#if 0       // Can't do this without patterns. It was a bit of a hack anyway
    int hiVal, loVal, reg, locals;
    if ((InstructionPatterns::new_reg_win(prog.csrSrc,hostPC, locals)) != NULL)
            return true;
    if ((InstructionPatterns::new_reg_win_large(prog.csrSrc, hostPC,
        hiVal, loVal, reg)) != NULL)
            return true;
    if ((InstructionPatterns::same_reg_win(prog.csrSrc, hostPC, locals))
        != NULL)
            return true;
    if ((InstructionPatterns::same_reg_win_large(prog.csrSrc, hostPC,
        hiVal, loVal, reg)) != NULL)
            return true;
#endif

    return false;
}

/*==============================================================================
 * FUNCTION:      isRestore()
 * OVERVIEW:      Check to see if the instruction at the given offset is a
 *                  restore instruction
 * PARAMETERS:    hostPC - pointer to the code in question (host address)
 * RETURNS:       True if a match found
 *============================================================================*/
bool SparcDecoder::isRestore(ADDRESS hostPC) {


#line 698 "frontend/machine/sparc/decoder.m"
{ 
  dword MATCH_p = 
    
#line 698 "frontend/machine/sparc/decoder.m"
    hostPC
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 61 && 
      (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2)) { 
      unsigned a = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      unsigned b = addressToPC(MATCH_p);
      unsigned c = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      
#line 700 "frontend/machine/sparc/decoder.m"
      

                  unused(a);      // Suppress warning messages

                  unused(b);

                  unused(c);

                  return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_a0;  /*opt-block+*/
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    
#line 704 "frontend/machine/sparc/decoder.m"
    
                return false;

    
     
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 708 "frontend/machine/sparc/decoder.m"
}

 /**********************************
 * These are the fetch routines.
 **********************************/

/*==============================================================================
 * FUNCTION:        getDword
 * OVERVIEW:        Returns the double starting at the given address.
 * PARAMETERS:      lc - address at which to decode the double
 * RETURNS:         the decoded double
 *============================================================================*/
DWord SparcDecoder::getDword(ADDRESS lc)
{
  Byte* p = (Byte*)lc;
  return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}

/*==============================================================================
 * FUNCTION:       SparcDecoder::SparcDecoder
 * OVERVIEW:       
 * PARAMETERS:     None
 * RETURNS:        N/A
 *============================================================================*/
SparcDecoder::SparcDecoder() : NJMCDecoder()
{
  std::string file = Boomerang::get()->getProgPath() + "frontend/machine/sparc/sparc.ssl";
  RTLDict.readSSLFile(file.c_str());
}

// For now...
int SparcDecoder::decodeAssemblyInstruction(unsigned, int)
{ return 0; }



