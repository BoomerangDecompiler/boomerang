#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 2 "machine/sparc/disassembler.m"
/***************************************************************************//**
 * \file       disassembler.m
 * OVERVIEW:   Skeleton file for a disassembler of SPARC instructions. 
 *============================================================================*/

/*
 * $Revision$
 *
 *    Apr 01 - Cristina: Created
 * 18 May 01 - Mike: Slight mods to accomodate other disassemblers; moved
 *              default NJMCDecoder constructor to disasm.cc
 */

#include "global.h"
#include "decoder.h"
#include "sparc-names.h"
#include "BinaryFile.h"         // For SymbolByAddress()

// Globals in driver disasm.cc file
extern  char _assembly[81];


// Need to provide the fetch routines for this machine (32-bits only
// on SPARC)

/*
 * FUNCTION:        getDword
 * OVERVIEW:        Returns the double starting at the given address.
 * PARAMETERS:      lc - address at which to decode the double
 * \returns          the decoded double
 */

DWord getDword(ADDRESS lc)
{
  Byte* p = (Byte*)lc;
  return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}



/* 
 * FUNCTION:         dis_RegImm
 * OVERVIEW:        decodes a register or an immediate value
 * PARAMETERS:         address pointer to be decoded
 * \returns          string with information about register or immediate 
 */

char *NJMCDecoder::dis_RegImm (ADDRESS pc)
{
    static char _buffer[11]; 



#line 54 "machine/sparc/disassembler.m"
{ 
  dword MATCH_p = 
    
    #line 54 "machine/sparc/disassembler.m"
    pc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
      int /* [~4096..4095] */ i = 
        sign_extend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
      
      #line 55 "machine/sparc/disassembler.m"
          sprintf (_buffer, "%d", i); 

      
      
      
    } /*opt-block*//*opt-block+*/
    else { 
      unsigned rs2 = (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */;
      
      #line 56 "machine/sparc/disassembler.m"
       sprintf (_buffer, "%s", DIS_RS2); 

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_c; 
  
  MATCH_finished_c: (void)0; /*placeholder for label*/
  
}

#line 59 "machine/sparc/disassembler.m"
    return _buffer;
}


/* 
 * FUNCTION:         dis_Eaddr
 * OVERVIEW:        decodes an effective address
 * PARAMETERS:         address pointer to be decoded
 * \returns          string with effective address in assembly format
 */

char* NJMCDecoder::dis_Eaddr (ADDRESS pc)
{
    static char _buffer[21]; 



#line 73 "machine/sparc/disassembler.m"
{ 
  dword MATCH_p = 
    
    #line 73 "machine/sparc/disassembler.m"
    pc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) 
      if ((MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0) { 
        int /* [~4096..4095] */ i = 
          sign_extend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
        
        #line 78 "machine/sparc/disassembler.m"
            sprintf (_buffer, "[0x%x]", i); 

                                strcat(constrName, "absoluteA ");

        
        
        
      } /*opt-block*//*opt-block+*/
      else { 
        int /* [~4096..4095] */ i = 
          sign_extend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
        unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
        
        #line 80 "machine/sparc/disassembler.m"
            sprintf (_buffer, "[%s+%d]", DIS_RS1, i); 

                                strcat(constrName, "dispA ");

        
        
        
      } /*opt-block*//*opt-block+*/ /*opt-block+*/
    else 
      if ((MATCH_w_32_0 & 0x1f) /* rs2 at 0 */ == 0) { 
        unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
        
        #line 74 "machine/sparc/disassembler.m"
            sprintf (_buffer, "[%s]", DIS_RS1);

                                strcat(constrName, "indirectA ");

        
        
        
      } /*opt-block*//*opt-block+*/
      else { 
        unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
        unsigned rs2 = (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */;
        
        #line 76 "machine/sparc/disassembler.m"
            sprintf (_buffer, "%s[%s]", DIS_RS1, DIS_RS2); 

                                strcat(constrName, "indexA ");

        
        
        
      } /*opt-block*//*opt-block+*/ /*opt-block+*/
    
  }goto MATCH_finished_b; 
  
  MATCH_finished_b: (void)0; /*placeholder for label*/
  
}

#line 84 "machine/sparc/disassembler.m"
    return _buffer;
}

/***************************************************************************//**
 * FUNCTION:       NJMCDecoder::decodeAssemblyInstruction
 * OVERVIEW:       Decodes a machine instruction and displays its assembly
 *                 representation onto the external array _assembly[].
 * PARAMETERS:     pc - the native address of the pc
 *                   delta - the difference between the native address and 
 *                    the host address of the pc
 * \returns         number of bytes taken up by the decoded instruction 
 *                    (i.e. number of bytes processed)
 *============================================================================*/

int NJMCDecoder::decodeAssemblyInstruction (ADDRESS pc, int delta)
{
    ADDRESS hostPC = pc + delta; 
    ADDRESS nextPC;

    sprintf(_assembly, "%X: %08X  ", pc, getDword(hostPC) );
    char* str = _assembly + strlen(_assembly);



#line 105 "machine/sparc/disassembler.m"
{ 
  dword MATCH_p = 
    
    #line 105 "machine/sparc/disassembler.m"
    hostPC
    ;
  char *MATCH_name;
  static char *MATCH_name_cond_1[] = {
    "BN", "BE", "BLE", "BL", "BLEU", "BCS", "BNEG", "BVS", "BA", "BNE", "BG", 
    "BGE", "BGU", "BCC", "BPOS", "BVC", 
  };
  static char *MATCH_name_cond_2[] = {
    "BN,a", "BE,a", "BLE,a", "BL,a", "BLEU,a", "BCS,a", "BNEG,a", "BVS,a", 
    "BA,a", "BNE,a", "BG,a", "BGE,a", "BGU,a", "BCC,a", "BPOS,a", "BVC,a", 
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
    "RDY", "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", 
    "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", (char *)0, "JMPL", 
    "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", 
    "JMPL", "JMPL", "JMPL", "JMPL", "JMPL", 
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
    (char *)0, (char *)0, (char *)0, (char *)0, "FSQRTs", (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, 
    (char *)0, (char *)0, (char *)0, (char *)0, "FADDs", "FADDd", "FADDq", 
    (char *)0, "FSUBs", "FSUBd", "FSUBq", (char *)0, "FMULs", "FMULd", 
    "FMULq", (char *)0, "FDIVs", "FDIVd", "FDIVq", (char *)0, "FCMPs", 
    "FCMPd", "FCMPq", (char *)0, "FCMPEs", "FCMPEd", "FCMPEq", (char *)0, 
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
    (char *)0, (char *)0, (char *)0, (char *)0, (char *)0, "FiTOs", 
    (char *)0, "FdTOs", "FqTOs", "FiTOd", "FsTOd", (char *)0, "FqTOd", 
    "FiTOq", "FsTOq", "FdTOq", (char *)0, (char *)0, "FsTOi", "FdTOi", 
    "FqTOi", 
  };
  static char *MATCH_name_i_50[] = {"JMPL", "RETT", };
  static char *MATCH_name_rs1_51[] = {
    "JMPL", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
    "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
    "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
    "SAVE", "SAVE", "SAVE", "SAVE", "SAVE", 
  };
  static char *MATCH_name_i_54[] = {"RETT", "SAVE", };
  static char *MATCH_name_cond_55[] = {
    "TN", "TE", "TLE", "TL", "TLEU", "TCS", "TNEG", "TVS", "TA", "TNE", "TG", 
    "TGE", "TGU", "TCC", "TPOS", "TVC", 
  };
  static char *MATCH_name_i_61[] = {"LD", "RESTORE", };
  static char *MATCH_name_i_62[] = {"LDUB", "LD", };
  static char *MATCH_name_i_63[] = {"LDUH", "LDUB", };
  static char *MATCH_name_i_64[] = {"LDD", "LDUH", };
  static char *MATCH_name_i_65[] = {"ST", "LDD", };
  static char *MATCH_name_i_66[] = {"STB", "ST", };
  static char *MATCH_name_i_67[] = {"STH", "STB", };
  static char *MATCH_name_i_68[] = {"STD", "STH", };
  static char *MATCH_name_i_69[] = {"LDSB", "STD", };
  static char *MATCH_name_i_70[] = {"LDSH", "LDSB", };
  static char *MATCH_name_i_71[] = {"LDSTUB", "LDSH", };
  static char *MATCH_name_i_72[] = {"SWAP.", "LDSTUB", };
  static char *MATCH_name_i_73[] = {"LDA", "SWAP.", };
  static char *MATCH_name_i_74[] = {"LDUBA", "LDF", };
  static char *MATCH_name_i_75[] = {"LDUHA", "LDFSR", };
  static char *MATCH_name_i_76[] = {"LDDA", "LDDF", };
  static char *MATCH_name_i_77[] = {"STA", "STF", };
  static char *MATCH_name_i_78[] = {"STBA", "STFSR", };
  static char *MATCH_name_i_79[] = {"STHA", "STDFQ", };
  static char *MATCH_name_i_80[] = {"STDA", "STDF", };
  static char *MATCH_name_i_81[] = {"LDSBA", "LDC", };
  static char *MATCH_name_i_82[] = {"LDSHA", "LDCSR", };
  static char *MATCH_name_i_83[] = {"LDSTUBA", "LDDC", };
  static char *MATCH_name_i_84[] = {"SWAPA", "STC", };
  static char *MATCH_name_i_85[] = {"LDF", "STCSR", };
  static char *MATCH_name_i_86[] = {"LDFSR", "STDCQ", };
  static char *MATCH_name_i_87[] = {"LDDF", "STDC", };
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    
      switch((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */) {
        case 0: 
          
            switch((MATCH_w_32_0 >> 22 & 0x7) /* op2 at 0 */) {
              case 0: 
                MATCH_name = "UNIMP"; 
                { 
                  char *name = MATCH_name;
                  unsigned n = (MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 276 "machine/sparc/disassembler.m"
                   

                          sprintf (str, "%s %d", name, n);

                  

                  
                  
                  
                }
                
                break;
              case 1: case 3: case 5: 
                goto MATCH_label_a0; break;
              case 2: 
                if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1 && 
                  (0 <= (MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ && 
                  (MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ < 16)) { 
                  MATCH_name = MATCH_name_cond_2[(MATCH_w_32_0 >> 25 & 0xf) 
                        /* cond at 0 */]; 
                  goto MATCH_label_a2; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = MATCH_name_cond_1[(MATCH_w_32_0 >> 25 & 0xf) 
                        /* cond at 0 */]; 
                  goto MATCH_label_a1; 
                  
                } /*opt-block*/
                
                break;
              case 4: 
                if ((MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */ == 0 && 
                  (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0) { 
                  nextPC = 4 + MATCH_p; 
                  
                  #line 108 "machine/sparc/disassembler.m"
                  

                          sprintf (str, "NOP");

                  

                  
                  
                  
                } /*opt-block*//*opt-block+*/
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
            
            #line 207 "machine/sparc/disassembler.m"
             {

                    // Get the actual destination

                    ADDRESS dest = tgt - delta;

                    // Get a symbol for it, if possible

                    const char* dsym = pBF->SymbolByAddress(dest);

                    char hexsym[128];

                    if (dsym == 0)

                        sprintf(hexsym, "0x%x", dest);

                     sprintf (str, "%s %s", "call", (dsym ? dsym : hexsym));

                }

            

            
            
            
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
                    
                    #line 174 "machine/sparc/disassembler.m"
                     

                            sprintf (str, "%s %s", name, DIS_RD);

                    

                    
                    
                    
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
                  
                  #line 177 "machine/sparc/disassembler.m"
                   

                          sprintf (str, "%s %s", name, DIS_RD);

                  

                  
                  
                  
                }
                
                break;
              case 42: 
                MATCH_name = MATCH_name_op3_44[(MATCH_w_32_0 >> 19 & 0x3f) 
                      /* op3 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 180 "machine/sparc/disassembler.m"
                   

                          sprintf (str, "%s %s", name, DIS_RD);

                  

                  
                  
                  
                }
                
                break;
              case 43: 
                MATCH_name = MATCH_name_op3_44[(MATCH_w_32_0 >> 19 & 0x3f) 
                      /* op3 at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 183 "machine/sparc/disassembler.m"
                   

                          sprintf (str, "%s %s", name, DIS_RD);

                  

                  
                  
                  
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
                    
                    #line 186 "machine/sparc/disassembler.m"
                     

                            sprintf (str, "%s %s,%s", name, DIS_RS1, DIS_ROI);

                    

                    
                    
                    
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
                  
                  #line 189 "machine/sparc/disassembler.m"
                   

                          sprintf (str, "%s %s,%s", name, DIS_RS1, DIS_ROI);

                  

                  
                  
                  
                }
                
                break;
              case 50: 
                MATCH_name = "WRWIM"; 
                { 
                  char *name = MATCH_name;
                  unsigned roi = addressToPC(MATCH_p);
                  unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 192 "machine/sparc/disassembler.m"
                   

                          sprintf (str, "%s %s,%s", name, DIS_RS1, DIS_ROI);

                  

                  
                  
                  
                }
                
                break;
              case 51: 
                MATCH_name = "WRTBR"; 
                { 
                  char *name = MATCH_name;
                  unsigned roi = addressToPC(MATCH_p);
                  unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 195 "machine/sparc/disassembler.m"
                   

                          sprintf (str, "%s %s,%s", name, DIS_RS1, DIS_ROI);

                  

                  
                  
                  
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
                    case 40: case 42: case 43: case 44: case 45: case 46: 
                    case 47: case 48: case 49: case 50: case 51: case 52: 
                    case 53: case 54: case 55: case 56: case 57: case 58: 
                    case 59: case 60: case 61: case 62: case 63: case 64: 
                    case 68: case 72: case 76: case 197: case 202: case 207: 
                    case 208: 
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
                        
                        #line 218 "machine/sparc/disassembler.m"
                         

                                sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDS);

                        

                        
                        
                        
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
                        
                        #line 221 "machine/sparc/disassembler.m"
                         

                                sprintf (str, "%s %s,%s,%s", name, DIS_FS1S, DIS_FS2S, DIS_FDS);

                         

                        
                        
                        
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
                        
                        #line 224 "machine/sparc/disassembler.m"
                         

                                sprintf (str, "%s %s,%s,%s", name, DIS_FS1D, DIS_FS2D, DIS_FDD);

                         

                        
                        
                        
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
                        
                        #line 227 "machine/sparc/disassembler.m"
                         

                                sprintf (str, "%s %s,%s,%s", name, DIS_FS1Q, DIS_FS2Q, DIS_FDQ);

                         

                        
                        
                        
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
                        
                        #line 239 "machine/sparc/disassembler.m"
                        

                                sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDS);

                        

                        
                        
                        
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
                        
                        #line 254 "machine/sparc/disassembler.m"
                        

                                sprintf (str, "%s %s,%s", name, DIS_FS2D, DIS_FDS);

                        

                        
                        
                        
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
                        
                        #line 259 "machine/sparc/disassembler.m"
                        

                                sprintf (str, "%s %s,%s", name, DIS_FS2Q, DIS_FDS);

                        

                        
                        
                        
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
                        
                        #line 241 "machine/sparc/disassembler.m"
                        

                                sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDD);

                        
                        
                        
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
                        
                        #line 251 "machine/sparc/disassembler.m"
                        

                                sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDD);

                        
                        
                        
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
                        
                        #line 264 "machine/sparc/disassembler.m"
                        

                                sprintf (str, "%s %s,%s", name, DIS_FS2Q, DIS_FDD);

                        

                        
                        
                        
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
                        
                        #line 246 "machine/sparc/disassembler.m"
                        

                                sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDQ);

                        
                        
                        
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
                        
                        #line 256 "machine/sparc/disassembler.m"
                        

                                sprintf (str, "%s %s,%s", name, DIS_FS2S, DIS_FDQ);

                        
                        
                        
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
                        
                        #line 261 "machine/sparc/disassembler.m"
                        

                                sprintf (str, "%s %s,%s", name, DIS_FS2D, DIS_FDQ);

                        
                        
                        
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
                        
                        #line 244 "machine/sparc/disassembler.m"
                        

                                sprintf (str, "%s %s,%s", name, DIS_FS2D, DIS_FDS);

                        

                        
                        
                        
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
                        
                        #line 249 "machine/sparc/disassembler.m"
                        

                                sprintf (str, "%s %s,%s", name, DIS_FS2Q, DIS_FDS);

                        

                        
                        
                        
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
                        
                        #line 230 "machine/sparc/disassembler.m"
                         

                                sprintf (str, "%s %s,%s", name, DIS_FS1S, DIS_FS2S);

                        

                        
                        
                        
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
                        
                        #line 233 "machine/sparc/disassembler.m"
                         

                                sprintf (str, "%s %s,%s", name, DIS_FS1D, DIS_FS2D);

                        

                        
                        
                        
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
                        
                        #line 236 "machine/sparc/disassembler.m"
                         

                                sprintf (str, "%s %s,%s", name, DIS_FS1Q, DIS_FS2Q);

                        

                        
                        
                        
                      }
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_32_0 >> 5 & 0x1ff) -- opf at 0 --*/ 
                break;
              case 56: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) 
                  
                    switch((MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_rs1_51[(MATCH_w_32_0 >> 14 & 0x1f) 
                              /* rs1 at 0 */]; 
                        goto MATCH_label_a5; 
                        
                        break;
                      case 1: case 2: case 3: case 4: case 5: case 6: case 7: 
                      case 8: case 9: case 10: case 11: case 12: case 13: 
                      case 14: case 16: case 17: case 18: case 19: case 20: 
                      case 21: case 22: case 23: case 24: case 25: case 26: 
                      case 27: case 28: case 29: case 30: 
                        MATCH_name = 
                          MATCH_name_rs1_43[(MATCH_w_32_0 >> 14 & 0x1f) 
                              /* rs1 at 0 */]; 
                        goto MATCH_label_a5; 
                        
                        break;
                      case 15: 
                        if ((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ == 8 && 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0) { 
                          nextPC = 4 + MATCH_p; 
                          
                          #line 120 "machine/sparc/disassembler.m"
                          

                                  sprintf (str, "retl");

                          

                          
                          
                          
                        } /*opt-block*//*opt-block+*/
                        else { 
                          MATCH_name = "JMPL"; 
                          goto MATCH_label_a5; 
                          
                        } /*opt-block*/
                        
                        break;
                      case 31: 
                        if ((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ == 8 && 
                          (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0) { 
                          nextPC = 4 + MATCH_p; 
                          
                          #line 117 "machine/sparc/disassembler.m"
                          

                                  sprintf (str, "ret");

                          

                          
                          
                          
                        } /*opt-block*//*opt-block+*/
                        else { 
                          MATCH_name = "JMPL"; 
                          goto MATCH_label_a5; 
                          
                        } /*opt-block*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_32_0 >> 14 & 0x1f) -- rs1 at 0 --*/  
                else { 
                  MATCH_name = 
                    MATCH_name_i_50[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a5; 
                  
                } /*opt-block*/
                break;
              case 57: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_54[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a6; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_50[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a6; 
                  
                } /*opt-block*/
                
                break;
              case 58: 
                MATCH_name = MATCH_name_cond_55[(MATCH_w_32_0 >> 25 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned addr = addressToPC(MATCH_p);
                  nextPC = 4 + MATCH_p; 
                  
                  #line 273 "machine/sparc/disassembler.m"
                   

                          sprintf (str, "%s %s", name, DIS_ADDR);

                  

                  
                  
                  
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
                          
                          #line 123 "machine/sparc/disassembler.m"
                          

                                  sprintf (str, "%s %s,%s", name, "%g0", "%g0", "%g0");

                          

                          
                          
                          
                        }
                        
                      } /*opt-block*/
                      else { 
                        MATCH_name = "SAVE"; 
                        goto MATCH_label_a4; 
                        
                      } /*opt-block*/ /*opt-block+*/
                    else { 
                      MATCH_name = 
                        MATCH_name_rs1_51[(MATCH_w_32_0 >> 14 & 0x1f) 
                            /* rs1 at 0 */]; 
                      goto MATCH_label_a4; 
                      
                    } /*opt-block*/ 
                  else { 
                    MATCH_name = "SAVE"; 
                    goto MATCH_label_a4; 
                    
                  } /*opt-block*/ 
                break;
              case 61: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_61[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a4; 
                  
                } /*opt-block*/
                else 
                  if ((MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0 && 
                    (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0 && 
                    (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */ == 0) { 
                    nextPC = 4 + MATCH_p; 
                    
                    #line 114 "machine/sparc/disassembler.m"
                    

                            sprintf (str, "restore");

                    

                    
                    
                    
                  } /*opt-block*//*opt-block+*/
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
                    MATCH_name_i_62[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_61[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                
                break;
              case 1: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_63[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_62[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                
                break;
              case 2: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_64[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_63[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                
                break;
              case 3: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_65[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_64[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                
                break;
              case 4: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_66[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_65[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
                } /*opt-block*/
                
                break;
              case 5: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_67[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_66[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
                } /*opt-block*/
                
                break;
              case 6: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_68[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_67[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
                } /*opt-block*/
                
                break;
              case 7: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_69[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_68[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a8; 
                  
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
                    MATCH_name_i_70[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_69[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                
                break;
              case 10: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_71[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_70[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                
                break;
              case 13: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_72[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_71[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                
                break;
              case 15: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_73[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_72[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a7; 
                  
                } /*opt-block*/
                
                break;
              case 16: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_73[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a9; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 17: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_74[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a9; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 18: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_75[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a9; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 19: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_76[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a9; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 20: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_77[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 21: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_78[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 22: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_79[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 23: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_80[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a10; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 25: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_81[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a9; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 26: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_82[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a9; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 29: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_83[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a9; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 31: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_84[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a9; 
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_a0;  /*opt-block+*/
                
                break;
              case 32: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_85[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a11; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_74[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a11; 
                  
                } /*opt-block*/
                
                break;
              case 33: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_86[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a12; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_75[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a12; 
                  
                } /*opt-block*/
                
                break;
              case 35: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) { 
                  MATCH_name = 
                    MATCH_name_i_87[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a13; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = 
                    MATCH_name_i_76[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a13; 
                  
                } /*opt-block*/
                
                break;
              case 36: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_77[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a14; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STF"; 
                  goto MATCH_label_a14; 
                  
                } /*opt-block*/
                
                break;
              case 37: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_78[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a15; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STFSR"; 
                  goto MATCH_label_a15; 
                  
                } /*opt-block*/
                
                break;
              case 38: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_79[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a16; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STDFQ"; 
                  goto MATCH_label_a16; 
                  
                } /*opt-block*/
                
                break;
              case 39: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_80[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a17; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STDF"; 
                  goto MATCH_label_a17; 
                  
                } /*opt-block*/
                
                break;
              case 48: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_81[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a18; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "LDC"; 
                  goto MATCH_label_a18; 
                  
                } /*opt-block*/
                
                break;
              case 49: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_82[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a19; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "LDCSR"; 
                  goto MATCH_label_a19; 
                  
                } /*opt-block*/
                
                break;
              case 51: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_83[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a18; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "LDDC"; 
                  goto MATCH_label_a18; 
                  
                } /*opt-block*/
                
                break;
              case 52: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_84[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a20; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STC"; 
                  goto MATCH_label_a20; 
                  
                } /*opt-block*/
                
                break;
              case 53: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_85[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a21; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STCSR"; 
                  goto MATCH_label_a21; 
                  
                } /*opt-block*/
                
                break;
              case 54: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_86[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a22; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STDCQ"; 
                  goto MATCH_label_a22; 
                  
                } /*opt-block*/
                
                break;
              case 55: 
                if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
                  MATCH_name = 
                    MATCH_name_i_87[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */]; 
                  goto MATCH_label_a20; 
                  
                } /*opt-block*/
                else { 
                  MATCH_name = "STDC"; 
                  goto MATCH_label_a20; 
                  
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
      
      #line 279 "machine/sparc/disassembler.m"
       

              // What does this mean?

              NULL;

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a1: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned tgt = 
        4 * sign_extend((MATCH_w_32_0 & 0x3fffff) /* disp22 at 0 */, 22) + 
        addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 204 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %X", name, tgt-delta);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a2: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned tgt = 
        4 * sign_extend((MATCH_w_32_0 & 0x3fffff) /* disp22 at 0 */, 22) + 
        addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 201 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %X", name, tgt-delta);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a3: (void)0; /*placeholder for label*/ 
    { 
      unsigned imm22 = (MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */ << 10;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 111 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s 0x%X,%s", "sethi", (imm22), DIS_RD);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a4: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      unsigned roi = addressToPC(MATCH_p);
      unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 198 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s,%s", name, DIS_RS1, DIS_ROI, DIS_RD);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a5: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 267 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s", name, DIS_ADDR, DIS_RD);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a6: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 270 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s", name);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a7: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 126 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s", name, DIS_ADDR, DIS_RD);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a8: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 141 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s", name, DIS_RD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a9: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned asi = (MATCH_w_32_0 >> 5 & 0xff) /* asi at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 138 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s", name, DIS_RD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a10: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned asi = (MATCH_w_32_0 >> 5 & 0xff) /* asi at 0 */;
      unsigned rd = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 153 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s", name, DIS_RD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a11: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fds = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 129 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s", name, DIS_ADDR, DIS_FDS);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a12: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 156 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s", name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a13: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fdd = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 132 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s", name, DIS_ADDR, DIS_FDD);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a14: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fds = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 144 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s", name, DIS_FDS, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a15: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 162 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s", name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a16: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 168 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s", name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a17: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned fdd = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 147 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s", name, DIS_FDD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a18: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned cd = (MATCH_w_32_0 >> 25 & 0x1f) /* cd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 135 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s", name, DIS_ADDR, DIS_CD);

              

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a19: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 159 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s", name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a20: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      unsigned cd = (MATCH_w_32_0 >> 25 & 0x1f) /* cd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 150 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s,%s", name, DIS_CD, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a21: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 165 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s", name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a22: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned addr = addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 171 "machine/sparc/disassembler.m"
       

              sprintf (str, "%s %s", name, DIS_ADDR);

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 287 "machine/sparc/disassembler.m"

    return (nextPC - hostPC);
}



