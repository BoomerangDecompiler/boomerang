#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 2 "machine/pentium/386.pat.m"
/*==============================================
 * FILE:      386.pat.m
 * OVERVIEW:  Generated file; do not edit
 *
 * (C) 1998-2000 The University of Queensland, BT group
 *==============================================*/

#include "global.h"
#include "decoder.h"
#include "386.pat.h"
#include "ss.h"
#include "csr.h"

#define VAR true
#define VAL false
int InstructionPatterns::EAX = 0;
int InstructionPatterns::ECX = 1;
int InstructionPatterns::EDX = 2;
int InstructionPatterns::EBX = 3;
int InstructionPatterns::ESP = 4;
int InstructionPatterns::EBP = 5;
int InstructionPatterns::ESI = 6;
int InstructionPatterns::EDI = 7;
int InstructionPatterns::iterhlp0(ADDRESS& lc, int& regs) {
	{ for(regs = 0; (PUSHod(lc, ESI, VAL) || 
	PUSHod(lc, EBX, VAL) || 
	PUSHod(lc, EDI, VAL)); regs++); return (regs >= 1 && 3 >= regs); }
return 0;
}

int InstructionPatterns::iterhlp1(ADDRESS& lc, int& regs) {
	{ for(regs = 0; (PUSHod(lc, ESI, VAL) || 
	PUSHod(lc, EBX, VAL) || 
	PUSHod(lc, EDI, VAL)); regs++); return (regs >= 1 && 3 >= regs); }
return 0;
}

int InstructionPatterns::iterhlp2(ADDRESS& lc, int& __loop, int& __loc0) {
	{ for(__loop = 0; (MOVrmod$E$Disp8(lc, EBX, VAL, __loc0, VAR, EBP, VAL) || 
	MOVrmod$E$Disp8(lc, ESI, VAL, __loc0, VAR, EBP, VAL) || 
	MOVrmod$E$Disp8(lc, EDI, VAL, __loc0, VAR, EBP, VAL)); __loop++); return (__loop >= 1 && 3 >= __loop); }
return 0;
}

int InstructionPatterns::iterhlp3(ADDRESS& lc, int& __loop, int& __loc0) {
	{ for(__loop = 0; (POPod(lc, EBX, VAL) || 
	POPod(lc, ESI, VAL) || 
	POPod(lc, EDI, VAL)); __loop++); return (__loop >= 1 && 3 >= __loop); }
return 0;
}

int InstructionPatterns::iterhlp4(ADDRESS& lc, int& __loop) {
	{ for(__loop = 0; (POPod(lc, EBX, VAL) || 
	POPod(lc, ESI, VAL) || 
	POPod(lc, EDI, VAL)); __loop++); return (__loop >= 1 && 3 >= __loop); }
return 0;
}

int InstructionPatterns::iterhlp5(ADDRESS& lc, int& n) {
	{ for(n = 0; (POPod(lc, EAX, VAL) || 
	POPod(lc, EBX, VAL) || 
	POPod(lc, ECX, VAL) || 
	POPod(lc, EDX, VAL) || 
	POPod(lc, ESI, VAL) || 
	POPod(lc, EDI, VAL) || 
	POPod(lc, EBP, VAL)); n++); return (n >= 1 && 7 >= n); }
return 0;
}

bool InstructionPatterns::ADDid$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 72 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 72 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  unsigned MATCH_w_32_16;
  unsigned MATCH_w_32_24;
  unsigned MATCH_w_32_32;
  unsigned MATCH_w_32_48;
  unsigned MATCH_w_32_56;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 1) 
      if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 8) 
        if ((MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) 
          goto MATCH_label_eb0;  /*opt-block+*/
        else { 
          MATCH_w_8_8 = getByte(1 + MATCH_p); 
          if ((MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */ == 0) 
            
              switch((MATCH_w_8_8 >> 6 & 0x3) /* mod at 8 */) {
                case 0: 
                  
                    switch((MATCH_w_8_8 & 0x7) /* r_m at 8 */) {
                      case 0: case 1: case 2: case 3: case 6: case 7: 
                        MATCH_w_32_16 = getDword(2 + MATCH_p); 
                        goto MATCH_label_eb1; 
                        
                        break;
                      case 4: 
                        MATCH_w_8_16 = getByte(2 + MATCH_p); 
                        if ((MATCH_w_8_16 & 0x7) /* base at 16 */ == 5 && 
                          (0 <= (MATCH_w_8_16 >> 3 & 0x7) 
                                /* index at 16 */ && 
                          (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                          MATCH_w_32_56 = getDword(7 + MATCH_p); 
                          goto MATCH_label_eb3; 
                          
                        } /*opt-block*/
                        else { 
                          MATCH_w_32_24 = getDword(3 + MATCH_p); 
                          goto MATCH_label_eb2; 
                          
                        } /*opt-block*/
                        
                        break;
                      case 5: 
                        MATCH_w_32_48 = getDword(6 + MATCH_p); 
                        goto MATCH_label_eb4; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_8_8 & 0x7) -- r_m at 8 --*/ 
                  break;
                case 1: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                    (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                    (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                    MATCH_w_32_32 = getDword(4 + MATCH_p); 
                    { 
                      unsigned _a = 1 + addressToPC(MATCH_p);
                      unsigned _b = MATCH_w_32_32 /* i32 at 32 */;
                      nextPC = 8 + MATCH_p; 
                      
                      #line 74 "machine/pentium/386.pat.m"
                      

                      		if (!Reg(_a, a, a_isVAR)) return false;

                      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

                      		lc = nextPC;

                      		return true;

                      
                      
                      
                    }
                    
                  } /*opt-block*/
                  else { 
                    MATCH_w_32_24 = getDword(3 + MATCH_p); 
                    goto MATCH_label_eb2; 
                    
                  } /*opt-block*/
                  
                  break;
                case 2: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                    (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                    (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                    MATCH_w_32_56 = getDword(7 + MATCH_p); 
                    goto MATCH_label_eb3; 
                    
                  } /*opt-block*/
                  else { 
                    MATCH_w_32_48 = getDword(6 + MATCH_p); 
                    goto MATCH_label_eb4; 
                    
                  } /*opt-block*/
                  
                  break;
                case 3: 
                  MATCH_w_32_16 = getDword(2 + MATCH_p); 
                  goto MATCH_label_eb1; 
                  
                  break;
                default: assert(0);
              } /* (MATCH_w_8_8 >> 6 & 0x3) -- mod at 8 --*/  
          else 
            goto MATCH_label_eb0;  /*opt-block+*/
          
        } /*opt-block*/ 
      else 
        goto MATCH_label_eb0;  /*opt-block+*/ 
    else 
      goto MATCH_label_eb0;  /*opt-block+*/
    
  }goto MATCH_finished_eb; 
  
  MATCH_label_eb0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 78 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_eb; 
    
  MATCH_label_eb1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = MATCH_w_32_16 /* i32 at 16 */;
      nextPC = 6 + MATCH_p; 
      
      #line 74 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_eb; 
    
  MATCH_label_eb2: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = MATCH_w_32_24 /* i32 at 24 */;
      nextPC = 7 + MATCH_p; 
      
      #line 74 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_eb; 
    
  MATCH_label_eb3: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = MATCH_w_32_56 /* i32 at 56 */;
      nextPC = 11 + MATCH_p; 
      
      #line 74 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_eb; 
    
  MATCH_label_eb4: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = MATCH_w_32_48 /* i32 at 48 */;
      nextPC = 10 + MATCH_p; 
      
      #line 74 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_eb; 
    
  MATCH_finished_eb: (void)0; /*placeholder for label*/
  
}

#line 82 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::ADDiodb$E$Base8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 84 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 84 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  unsigned /* [0..255] */ MATCH_w_8_24;
  unsigned /* [0..255] */ MATCH_w_8_32;
  unsigned /* [0..255] */ MATCH_w_8_48;
  unsigned /* [0..255] */ MATCH_w_8_56;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 3) 
      if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 8) 
        if ((MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) 
          goto MATCH_label_db0;  /*opt-block+*/
        else { 
          MATCH_w_8_8 = getByte(1 + MATCH_p); 
          if ((MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */ == 0) 
            
              switch((MATCH_w_8_8 >> 6 & 0x3) /* mod at 8 */) {
                case 0: 
                  
                    switch((MATCH_w_8_8 & 0x7) /* r_m at 8 */) {
                      case 0: case 1: case 2: case 3: case 6: case 7: 
                        MATCH_w_8_16 = getByte(2 + MATCH_p); 
                        goto MATCH_label_db1; 
                        
                        break;
                      case 4: 
                        MATCH_w_8_16 = getByte(2 + MATCH_p); 
                        if ((MATCH_w_8_16 & 0x7) /* base at 16 */ == 5 && 
                          (0 <= (MATCH_w_8_16 >> 3 & 0x7) 
                                /* index at 16 */ && 
                          (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                          MATCH_w_8_56 = getByte(7 + MATCH_p); 
                          goto MATCH_label_db3; 
                          
                        } /*opt-block*/
                        else { 
                          MATCH_w_8_24 = getByte(3 + MATCH_p); 
                          goto MATCH_label_db2; 
                          
                        } /*opt-block*/
                        
                        break;
                      case 5: 
                        MATCH_w_8_48 = getByte(6 + MATCH_p); 
                        goto MATCH_label_db4; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_8_8 & 0x7) -- r_m at 8 --*/ 
                  break;
                case 1: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                    (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                    (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                    MATCH_w_8_32 = getByte(4 + MATCH_p); 
                    { 
                      unsigned _a = 1 + addressToPC(MATCH_p);
                      int /* [~128..127] */ _b = 
                        sign_extend((MATCH_w_8_32 & 0xff) /* i8 at 32 */, 8);
                      nextPC = 5 + MATCH_p; 
                      
                      #line 86 "machine/pentium/386.pat.m"
                      

                      		if (!E$Base8(_a, a, a_isVAR, b, b_isVAR)) return false;

                      		if (!c_isVAR && (int)_b != c) return false; else c = _b;

                      		lc = nextPC;

                      		return true;

                      
                      
                      
                    }
                    
                  } /*opt-block*/
                  else { 
                    MATCH_w_8_24 = getByte(3 + MATCH_p); 
                    goto MATCH_label_db2; 
                    
                  } /*opt-block*/
                  
                  break;
                case 2: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                    (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                    (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                    MATCH_w_8_56 = getByte(7 + MATCH_p); 
                    goto MATCH_label_db3; 
                    
                  } /*opt-block*/
                  else { 
                    MATCH_w_8_48 = getByte(6 + MATCH_p); 
                    goto MATCH_label_db4; 
                    
                  } /*opt-block*/
                  
                  break;
                case 3: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  goto MATCH_label_db1; 
                  
                  break;
                default: assert(0);
              } /* (MATCH_w_8_8 >> 6 & 0x3) -- mod at 8 --*/  
          else 
            goto MATCH_label_db0;  /*opt-block+*/
          
        } /*opt-block*/ 
      else 
        goto MATCH_label_db0;  /*opt-block+*/ 
    else 
      goto MATCH_label_db0;  /*opt-block+*/
    
  }goto MATCH_finished_db; 
  
  MATCH_label_db0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 90 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_db; 
    
  MATCH_label_db1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_16 & 0xff) /* i8 at 16 */, 8);
      nextPC = 3 + MATCH_p; 
      
      #line 86 "machine/pentium/386.pat.m"
      

      		if (!E$Base8(_a, a, a_isVAR, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_b != c) return false; else c = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_db; 
    
  MATCH_label_db2: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_24 & 0xff) /* i8 at 24 */, 8);
      nextPC = 4 + MATCH_p; 
      
      #line 86 "machine/pentium/386.pat.m"
      

      		if (!E$Base8(_a, a, a_isVAR, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_b != c) return false; else c = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_db; 
    
  MATCH_label_db3: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_56 & 0xff) /* i8 at 56 */, 8);
      nextPC = 8 + MATCH_p; 
      
      #line 86 "machine/pentium/386.pat.m"
      

      		if (!E$Base8(_a, a, a_isVAR, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_b != c) return false; else c = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_db; 
    
  MATCH_label_db4: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_48 & 0xff) /* i8 at 48 */, 8);
      nextPC = 7 + MATCH_p; 
      
      #line 86 "machine/pentium/386.pat.m"
      

      		if (!E$Base8(_a, a, a_isVAR, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_b != c) return false; else c = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_db; 
    
  MATCH_finished_db: (void)0; /*placeholder for label*/
  
}

#line 94 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::ADDiodb$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 96 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 96 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  unsigned /* [0..255] */ MATCH_w_8_24;
  unsigned /* [0..255] */ MATCH_w_8_32;
  unsigned /* [0..255] */ MATCH_w_8_48;
  unsigned /* [0..255] */ MATCH_w_8_56;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 3) 
      if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 8) 
        if ((MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) 
          goto MATCH_label_cb0;  /*opt-block+*/
        else { 
          MATCH_w_8_8 = getByte(1 + MATCH_p); 
          if ((MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */ == 0) 
            
              switch((MATCH_w_8_8 >> 6 & 0x3) /* mod at 8 */) {
                case 0: 
                  
                    switch((MATCH_w_8_8 & 0x7) /* r_m at 8 */) {
                      case 0: case 1: case 2: case 3: case 6: case 7: 
                        MATCH_w_8_16 = getByte(2 + MATCH_p); 
                        goto MATCH_label_cb1; 
                        
                        break;
                      case 4: 
                        MATCH_w_8_16 = getByte(2 + MATCH_p); 
                        if ((MATCH_w_8_16 & 0x7) /* base at 16 */ == 5 && 
                          (0 <= (MATCH_w_8_16 >> 3 & 0x7) 
                                /* index at 16 */ && 
                          (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                          MATCH_w_8_56 = getByte(7 + MATCH_p); 
                          goto MATCH_label_cb3; 
                          
                        } /*opt-block*/
                        else { 
                          MATCH_w_8_24 = getByte(3 + MATCH_p); 
                          goto MATCH_label_cb2; 
                          
                        } /*opt-block*/
                        
                        break;
                      case 5: 
                        MATCH_w_8_48 = getByte(6 + MATCH_p); 
                        goto MATCH_label_cb4; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_8_8 & 0x7) -- r_m at 8 --*/ 
                  break;
                case 1: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                    (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                    (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                    MATCH_w_8_32 = getByte(4 + MATCH_p); 
                    { 
                      unsigned _a = 1 + addressToPC(MATCH_p);
                      int /* [~128..127] */ _b = 
                        sign_extend((MATCH_w_8_32 & 0xff) /* i8 at 32 */, 8);
                      nextPC = 5 + MATCH_p; 
                      
                      #line 98 "machine/pentium/386.pat.m"
                      

                      		if (!Reg(_a, a, a_isVAR)) return false;

                      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

                      		lc = nextPC;

                      		return true;

                      
                      
                      
                    }
                    
                  } /*opt-block*/
                  else { 
                    MATCH_w_8_24 = getByte(3 + MATCH_p); 
                    goto MATCH_label_cb2; 
                    
                  } /*opt-block*/
                  
                  break;
                case 2: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                    (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                    (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                    MATCH_w_8_56 = getByte(7 + MATCH_p); 
                    goto MATCH_label_cb3; 
                    
                  } /*opt-block*/
                  else { 
                    MATCH_w_8_48 = getByte(6 + MATCH_p); 
                    goto MATCH_label_cb4; 
                    
                  } /*opt-block*/
                  
                  break;
                case 3: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  goto MATCH_label_cb1; 
                  
                  break;
                default: assert(0);
              } /* (MATCH_w_8_8 >> 6 & 0x3) -- mod at 8 --*/  
          else 
            goto MATCH_label_cb0;  /*opt-block+*/
          
        } /*opt-block*/ 
      else 
        goto MATCH_label_cb0;  /*opt-block+*/ 
    else 
      goto MATCH_label_cb0;  /*opt-block+*/
    
  }goto MATCH_finished_cb; 
  
  MATCH_label_cb0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 102 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_cb; 
    
  MATCH_label_cb1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_16 & 0xff) /* i8 at 16 */, 8);
      nextPC = 3 + MATCH_p; 
      
      #line 98 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_cb; 
    
  MATCH_label_cb2: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_24 & 0xff) /* i8 at 24 */, 8);
      nextPC = 4 + MATCH_p; 
      
      #line 98 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_cb; 
    
  MATCH_label_cb3: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_56 & 0xff) /* i8 at 56 */, 8);
      nextPC = 8 + MATCH_p; 
      
      #line 98 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_cb; 
    
  MATCH_label_cb4: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_48 & 0xff) /* i8 at 48 */, 8);
      nextPC = 7 + MATCH_p; 
      
      #line 98 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_cb; 
    
  MATCH_finished_cb: (void)0; /*placeholder for label*/
  
}

#line 105 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::Base(ADDRESS& lc, int& a, bool a_isVAR) {


#line 107 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 107 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  { 
    MATCH_w_8_8 = getByte(1 + MATCH_p); 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if (0 <= (MATCH_w_8_0 & 0x7) /* r_m at 0 */ && (MATCH_w_8_0 & 0x7) 
            /* r_m at 0 */ < 4 || 5 <= (MATCH_w_8_0 & 0x7) /* r_m at 0 */ && 
      (MATCH_w_8_0 & 0x7) /* r_m at 0 */ < 8 || 
      (MATCH_w_8_0 & 0x7) /* r_m at 0 */ == 4 && 
      (MATCH_w_8_0 >> 6 & 0x3) /* mod at 0 */ == 0 && 
      (0 <= (MATCH_w_8_8 >> 3 & 0x7) /* index at 8 */ && 
      (MATCH_w_8_8 >> 3 & 0x7) /* index at 8 */ < 4 || 
      5 <= (MATCH_w_8_8 >> 3 & 0x7) /* index at 8 */ && 
      (MATCH_w_8_8 >> 3 & 0x7) /* index at 8 */ < 8) || 
      (MATCH_w_8_0 & 0x7) /* r_m at 0 */ == 4 && 
      (MATCH_w_8_0 >> 6 & 0x3) /* mod at 0 */ == 0 && 
      (MATCH_w_8_8 >> 3 & 0x7) /* index at 8 */ == 4 && 
      (MATCH_w_8_8 & 0x7) /* base at 8 */ == 5 || (MATCH_w_8_0 & 0x7) 
            /* r_m at 0 */ == 4 && (1 <= (MATCH_w_8_0 >> 6 & 0x3) 
            /* mod at 0 */ && (MATCH_w_8_0 >> 6 & 0x3) /* mod at 0 */ < 4)) 
      goto MATCH_label_bb0;  /*opt-block+*/
    else { 
      unsigned _a = (MATCH_w_8_8 & 0x7) /* base at 8 */;
      
      #line 109 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_bb; 
  
  MATCH_label_bb0: (void)0; /*placeholder for label*/ 
    
    #line 111 "machine/pentium/386.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_bb; 
    
  MATCH_finished_bb: (void)0; /*placeholder for label*/
  
}

#line 114 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::Base8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {


#line 116 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 116 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    MATCH_w_8_8 = getByte(1 + MATCH_p); 
    if ((MATCH_w_8_0 >> 6 & 0x3) /* mod at 0 */ == 1 && 
      (MATCH_w_8_0 & 0x7) /* r_m at 0 */ == 4 && 
      (MATCH_w_8_8 >> 3 & 0x7) /* index at 8 */ == 4) { 
      MATCH_w_8_16 = getByte(2 + MATCH_p); 
      { 
        unsigned _a = (MATCH_w_8_16 & 0xff) /* i8 at 16 */;
        unsigned _b = (MATCH_w_8_8 & 0x7) /* base at 8 */;
        
        #line 118 "machine/pentium/386.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		if (!b_isVAR && (int)_b != b) return false; else b = _b;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    else 
      goto MATCH_label_ab0;  /*opt-block+*/
    
  }goto MATCH_finished_ab; 
  
  MATCH_label_ab0: (void)0; /*placeholder for label*/ 
    
    #line 121 "machine/pentium/386.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_ab; 
    
  MATCH_finished_ab: (void)0; /*placeholder for label*/
  
}

#line 125 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::CALL$Jvod(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 127 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 127 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned MATCH_w_32_8;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 14 && 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 0 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) { 
      MATCH_w_32_8 = getDword(1 + MATCH_p); 
      { 
        unsigned _a = 5 + MATCH_w_32_8 /* i32 at 8 */ + addressToPC(MATCH_p);
        nextPC = 5 + MATCH_p; 
        
        #line 129 "machine/pentium/386.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    else 
      goto MATCH_label_z0;  /*opt-block+*/
    
  }goto MATCH_finished_z; 
  
  MATCH_label_z0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 132 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_z; 
    
  MATCH_finished_z: (void)0; /*placeholder for label*/
  
}

#line 136 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::CLD(ADDRESS& lc) {
	ADDRESS nextPC;


#line 138 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 138 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 4 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 15) { 
      nextPC = 1 + MATCH_p; 
      
      #line 140 "machine/pentium/386.pat.m"
      

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_y0;  /*opt-block+*/
    
  }goto MATCH_finished_y; 
  
  MATCH_label_y0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 142 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_y; 
    
  MATCH_finished_y: (void)0; /*placeholder for label*/
  
}

#line 145 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {


#line 147 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 147 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 >> 6 & 0x3) /* mod at 0 */ == 0 || 
      2 <= (MATCH_w_8_0 >> 6 & 0x3) /* mod at 0 */ && 
      (MATCH_w_8_0 >> 6 & 0x3) /* mod at 0 */ < 4 || (MATCH_w_8_0 >> 6 & 0x3) 
            /* mod at 0 */ == 1 && (MATCH_w_8_0 & 0x7) /* r_m at 0 */ == 4) 
      goto MATCH_label_x0;  /*opt-block+*/
    else { 
      MATCH_w_8_8 = getByte(1 + MATCH_p); 
      { 
        int /* [~128..127] */ _a = 
          sign_extend((MATCH_w_8_8 & 0xff) /* i8 at 8 */, 8);
        unsigned _b = (MATCH_w_8_0 & 0x7) /* r_m at 0 */;
        
        #line 149 "machine/pentium/386.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		if (!b_isVAR && (int)_b != b) return false; else b = _b;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    
  }goto MATCH_finished_x; 
  
  MATCH_label_x0: (void)0; /*placeholder for label*/ 
    
    #line 152 "machine/pentium/386.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_x; 
    
  MATCH_finished_x: (void)0; /*placeholder for label*/
  
}

#line 155 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::E$Base(ADDRESS& lc, int& a, bool a_isVAR) {


#line 157 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 157 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 >> 6 & 0x3) /* mod at 0 */ == 3) 
      
      #line 161 "machine/pentium/386.pat.m"
      
      		return false;

      
       /*opt-block+*/
    else 
      goto MATCH_label_w0;  /*opt-block+*/
    
  }goto MATCH_finished_w; 
  
  MATCH_label_w0: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = addressToPC(MATCH_p);
      
      #line 159 "machine/pentium/386.pat.m"
      

      		if (!Base(_a, a, a_isVAR)) return false;

      		return true;

      
      
      
    } 
    goto MATCH_finished_w; 
    
  MATCH_finished_w: (void)0; /*placeholder for label*/
  
}

#line 164 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::E$Base8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {


#line 166 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 166 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 >> 6 & 0x3) /* mod at 0 */ == 3) 
      
      #line 170 "machine/pentium/386.pat.m"
      
      		return false;

      
       /*opt-block+*/
    else 
      goto MATCH_label_v0;  /*opt-block+*/
    
  }goto MATCH_finished_v; 
  
  MATCH_label_v0: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = addressToPC(MATCH_p);
      
      #line 168 "machine/pentium/386.pat.m"
      

      		if (!Base8(_a, a, a_isVAR, b, b_isVAR)) return false;

      		return true;

      
      
      
    } 
    goto MATCH_finished_v; 
    
  MATCH_finished_v: (void)0; /*placeholder for label*/
  
}

#line 173 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::E$Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {


#line 175 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 175 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 >> 6 & 0x3) /* mod at 0 */ == 3) 
      
      #line 179 "machine/pentium/386.pat.m"
      
      		return false;

      
       /*opt-block+*/
    else 
      goto MATCH_label_u0;  /*opt-block+*/
    
  }goto MATCH_finished_u; 
  
  MATCH_label_u0: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = addressToPC(MATCH_p);
      
      #line 177 "machine/pentium/386.pat.m"
      

      		if (!Disp8(_a, a, a_isVAR, b, b_isVAR)) return false;

      		return true;

      
      
      
    } 
    goto MATCH_finished_u; 
    
  MATCH_finished_u: (void)0; /*placeholder for label*/
  
}

#line 183 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::JMP$Jvod(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 185 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 185 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned MATCH_w_32_8;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 1 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 14 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) { 
      MATCH_w_32_8 = getDword(1 + MATCH_p); 
      { 
        unsigned _a = 5 + MATCH_w_32_8 /* i32 at 8 */ + addressToPC(MATCH_p);
        nextPC = 5 + MATCH_p; 
        
        #line 187 "machine/pentium/386.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    else 
      goto MATCH_label_t0;  /*opt-block+*/
    
  }goto MATCH_finished_t; 
  
  MATCH_label_t0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 190 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_t; 
    
  MATCH_finished_t: (void)0; /*placeholder for label*/
  
}

#line 194 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::LEAVE(ADDRESS& lc) {
	ADDRESS nextPC;


#line 196 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 196 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 1 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 12 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) { 
      nextPC = 1 + MATCH_p; 
      
      #line 198 "machine/pentium/386.pat.m"
      

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_s0;  /*opt-block+*/
    
  }goto MATCH_finished_s; 
  
  MATCH_label_s0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 200 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_s; 
    
  MATCH_finished_s: (void)0; /*placeholder for label*/
  
}

#line 204 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::LEAod$Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 206 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 206 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 5) 
      if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 8) 
        if ((MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) { 
          MATCH_w_8_8 = getByte(1 + MATCH_p); 
          
            switch((MATCH_w_8_8 >> 6 & 0x3) /* mod at 8 */) {
              case 0: 
                
                  switch((MATCH_w_8_8 & 0x7) /* r_m at 8 */) {
                    case 0: case 1: case 2: case 3: case 6: case 7: 
                      { 
                        unsigned _a = 
                          (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
                        unsigned _b = 1 + addressToPC(MATCH_p);
                        nextPC = 2 + MATCH_p; 
                        
                        #line 208 "machine/pentium/386.pat.m"
                        

                        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

                        		if (!Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;

                        		lc = nextPC;

                        		return true;

                        
                        
                        
                      }
                      
                      break;
                    case 4: 
                      MATCH_w_8_16 = getByte(2 + MATCH_p); 
                      if ((MATCH_w_8_16 & 0x7) /* base at 16 */ == 5 && 
                        (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                        (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                        goto MATCH_label_r2;  /*opt-block+*/
                      else 
                        goto MATCH_label_r1;  /*opt-block+*/
                      
                      break;
                    case 5: 
                      goto MATCH_label_r3; break;
                    default: assert(0);
                  } /* (MATCH_w_8_8 & 0x7) -- r_m at 8 --*/ 
                break;
              case 1: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                  unsigned _a = 
                    (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
                  unsigned _b = 1 + addressToPC(MATCH_p);
                  nextPC = 4 + MATCH_p; 
                  
                  #line 208 "machine/pentium/386.pat.m"
                  

                  		if (!a_isVAR && (int)_a != a) return false; else a = _a;

                  		if (!Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;

                  		lc = nextPC;

                  		return true;

                  
                  
                  
                } /*opt-block*//*opt-block+*/
                else 
                  goto MATCH_label_r1;  /*opt-block+*/
                
                break;
              case 2: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                  goto MATCH_label_r2;  /*opt-block+*/
                else 
                  goto MATCH_label_r3;  /*opt-block+*/
                
                break;
              case 3: 
                goto MATCH_label_r0; break;
              default: assert(0);
            } /* (MATCH_w_8_8 >> 6 & 0x3) -- mod at 8 --*/ 
          
        } /*opt-block*/
        else 
          goto MATCH_label_r0;  /*opt-block+*/ 
      else 
        goto MATCH_label_r0;  /*opt-block+*/ 
    else 
      goto MATCH_label_r0;  /*opt-block+*/
    
  }goto MATCH_finished_r; 
  
  MATCH_label_r0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 212 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_r; 
    
  MATCH_label_r1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 3 + MATCH_p; 
      
      #line 208 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_r; 
    
  MATCH_label_r2: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 7 + MATCH_p; 
      
      #line 208 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_r; 
    
  MATCH_label_r3: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 6 + MATCH_p; 
      
      #line 208 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_r; 
    
  MATCH_finished_r: (void)0; /*placeholder for label*/
  
}

#line 216 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::MOVSB(ADDRESS& lc) {
	ADDRESS nextPC;


#line 218 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 218 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 4 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 10 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0) { 
      nextPC = 1 + MATCH_p; 
      
      #line 220 "machine/pentium/386.pat.m"
      

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_q0;  /*opt-block+*/
    
  }goto MATCH_finished_q; 
  
  MATCH_label_q0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 222 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_q; 
    
  MATCH_finished_q: (void)0; /*placeholder for label*/
  
}

#line 226 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::MOVSvow(ADDRESS& lc) {
	ADDRESS nextPC;


#line 228 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 228 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  { 
    MATCH_w_8_8 = getByte(1 + MATCH_p); 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if (0 <= (MATCH_w_8_0 & 0x7) /* col at 0 */ && (MATCH_w_8_0 & 0x7) 
            /* col at 0 */ < 6 || (MATCH_w_8_0 & 0x7) /* col at 0 */ == 7 || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 6 && 
      (0 <= (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ < 6 || 
      7 <= (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ < 16) || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 6 && (MATCH_w_8_0 >> 4 & 0xf) 
            /* row at 0 */ == 6 && (MATCH_w_8_0 >> 3 & 0x1) 
            /* page at 0 */ == 0 && (0 <= (MATCH_w_8_8 & 0x7) 
            /* col at 8 */ && (MATCH_w_8_8 & 0x7) /* col at 8 */ < 5 || 
      6 <= (MATCH_w_8_8 & 0x7) /* col at 8 */ && (MATCH_w_8_8 & 0x7) 
            /* col at 8 */ < 8) || (MATCH_w_8_0 & 0x7) /* col at 0 */ == 6 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 6 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0 && (MATCH_w_8_8 & 0x7) 
            /* col at 8 */ == 5 && (0 <= (MATCH_w_8_8 >> 4 & 0xf) 
            /* row at 8 */ && (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ < 10 || 
      11 <= (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ && 
      (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ < 16) || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 6 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 6 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0 && 
      (MATCH_w_8_8 & 0x7) /* col at 8 */ == 5 && 
      (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ == 10 && 
      (MATCH_w_8_8 >> 3 & 0x1) /* page at 8 */ == 1 || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 6 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 6 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) 
      goto MATCH_label_p0;  /*opt-block+*/
    else { 
      nextPC = 2 + MATCH_p; 
      
      #line 230 "machine/pentium/386.pat.m"
      

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_p; 
  
  MATCH_label_p0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 232 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_p; 
    
  MATCH_finished_p: (void)0; /*placeholder for label*/
  
}

#line 236 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::MOVid(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 238 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 238 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned MATCH_w_32_8;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 11 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) { 
      MATCH_w_32_8 = getDword(1 + MATCH_p); 
      { 
        unsigned _a = (MATCH_w_8_0 & 0x7) /* r32 at 0 */;
        unsigned _b = MATCH_w_32_8 /* i32 at 8 */;
        nextPC = 5 + MATCH_p; 
        
        #line 240 "machine/pentium/386.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		if (!b_isVAR && (int)_b != b) return false; else b = _b;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    else 
      goto MATCH_label_o0;  /*opt-block+*/
    
  }goto MATCH_finished_o; 
  
  MATCH_label_o0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 244 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_o; 
    
  MATCH_finished_o: (void)0; /*placeholder for label*/
  
}

#line 248 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::MOVmrod$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 250 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 250 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 1) 
      if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 8) 
        if ((MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) { 
          MATCH_w_8_8 = getByte(1 + MATCH_p); 
          
            switch((MATCH_w_8_8 >> 6 & 0x3) /* mod at 8 */) {
              case 0: 
                
                  switch((MATCH_w_8_8 & 0x7) /* r_m at 8 */) {
                    case 0: case 1: case 2: case 3: case 6: case 7: 
                      goto MATCH_label_n1; break;
                    case 4: 
                      MATCH_w_8_16 = getByte(2 + MATCH_p); 
                      if ((MATCH_w_8_16 & 0x7) /* base at 16 */ == 5 && 
                        (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                        (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                        goto MATCH_label_n3;  /*opt-block+*/
                      else 
                        goto MATCH_label_n2;  /*opt-block+*/
                      
                      break;
                    case 5: 
                      goto MATCH_label_n4; break;
                    default: assert(0);
                  } /* (MATCH_w_8_8 & 0x7) -- r_m at 8 --*/ 
                break;
              case 1: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                  unsigned _a = 1 + addressToPC(MATCH_p);
                  unsigned _b = 
                    (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 252 "machine/pentium/386.pat.m"
                  

                  		if (!Reg(_a, a, a_isVAR)) return false;

                  		if (!b_isVAR && (int)_b != b) return false; else b = _b;

                  		lc = nextPC;

                  		return true;

                  
                  
                  
                } /*opt-block*//*opt-block+*/
                else 
                  goto MATCH_label_n2;  /*opt-block+*/
                
                break;
              case 2: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                  goto MATCH_label_n3;  /*opt-block+*/
                else 
                  goto MATCH_label_n4;  /*opt-block+*/
                
                break;
              case 3: 
                goto MATCH_label_n1; break;
              default: assert(0);
            } /* (MATCH_w_8_8 >> 6 & 0x3) -- mod at 8 --*/ 
          
        } /*opt-block*/
        else 
          goto MATCH_label_n0;  /*opt-block+*/ 
      else 
        goto MATCH_label_n0;  /*opt-block+*/ 
    else 
      goto MATCH_label_n0;  /*opt-block+*/
    
  }goto MATCH_finished_n; 
  
  MATCH_label_n0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 256 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_n; 
    
  MATCH_label_n1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      nextPC = 2 + MATCH_p; 
      
      #line 252 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_n; 
    
  MATCH_label_n2: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      nextPC = 3 + MATCH_p; 
      
      #line 252 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_n; 
    
  MATCH_label_n3: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      nextPC = 7 + MATCH_p; 
      
      #line 252 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_n; 
    
  MATCH_label_n4: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      nextPC = 6 + MATCH_p; 
      
      #line 252 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_n; 
    
  MATCH_finished_n: (void)0; /*placeholder for label*/
  
}

#line 260 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::MOVrmod$E$Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 262 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 262 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 3) 
      if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 8) 
        if ((MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) { 
          MATCH_w_8_8 = getByte(1 + MATCH_p); 
          
            switch((MATCH_w_8_8 >> 6 & 0x3) /* mod at 8 */) {
              case 0: 
                
                  switch((MATCH_w_8_8 & 0x7) /* r_m at 8 */) {
                    case 0: case 1: case 2: case 3: case 6: case 7: 
                      goto MATCH_label_m1; break;
                    case 4: 
                      MATCH_w_8_16 = getByte(2 + MATCH_p); 
                      if ((MATCH_w_8_16 & 0x7) /* base at 16 */ == 5 && 
                        (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                        (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                        goto MATCH_label_m3;  /*opt-block+*/
                      else 
                        goto MATCH_label_m2;  /*opt-block+*/
                      
                      break;
                    case 5: 
                      goto MATCH_label_m4; break;
                    default: assert(0);
                  } /* (MATCH_w_8_8 & 0x7) -- r_m at 8 --*/ 
                break;
              case 1: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                  unsigned _a = 
                    (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
                  unsigned _b = 1 + addressToPC(MATCH_p);
                  nextPC = 4 + MATCH_p; 
                  
                  #line 264 "machine/pentium/386.pat.m"
                  

                  		if (!a_isVAR && (int)_a != a) return false; else a = _a;

                  		if (!E$Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;

                  		lc = nextPC;

                  		return true;

                  
                  
                  
                } /*opt-block*//*opt-block+*/
                else 
                  goto MATCH_label_m2;  /*opt-block+*/
                
                break;
              case 2: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                  goto MATCH_label_m3;  /*opt-block+*/
                else 
                  goto MATCH_label_m4;  /*opt-block+*/
                
                break;
              case 3: 
                goto MATCH_label_m1; break;
              default: assert(0);
            } /* (MATCH_w_8_8 >> 6 & 0x3) -- mod at 8 --*/ 
          
        } /*opt-block*/
        else 
          goto MATCH_label_m0;  /*opt-block+*/ 
      else 
        goto MATCH_label_m0;  /*opt-block+*/ 
    else 
      goto MATCH_label_m0;  /*opt-block+*/
    
  }goto MATCH_finished_m; 
  
  MATCH_label_m0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 268 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_m; 
    
  MATCH_label_m1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 2 + MATCH_p; 
      
      #line 264 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!E$Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_m; 
    
  MATCH_label_m2: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 3 + MATCH_p; 
      
      #line 264 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!E$Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_m; 
    
  MATCH_label_m3: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 7 + MATCH_p; 
      
      #line 264 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!E$Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_m; 
    
  MATCH_label_m4: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 6 + MATCH_p; 
      
      #line 264 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!E$Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_m; 
    
  MATCH_finished_m: (void)0; /*placeholder for label*/
  
}

#line 272 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::MOVrmod$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 274 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 274 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 3) 
      if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 8) 
        if ((MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) { 
          MATCH_w_8_8 = getByte(1 + MATCH_p); 
          
            switch((MATCH_w_8_8 >> 6 & 0x3) /* mod at 8 */) {
              case 0: 
                
                  switch((MATCH_w_8_8 & 0x7) /* r_m at 8 */) {
                    case 0: case 1: case 2: case 3: case 6: case 7: 
                      goto MATCH_label_l1; break;
                    case 4: 
                      MATCH_w_8_16 = getByte(2 + MATCH_p); 
                      if ((MATCH_w_8_16 & 0x7) /* base at 16 */ == 5 && 
                        (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                        (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                        goto MATCH_label_l3;  /*opt-block+*/
                      else 
                        goto MATCH_label_l2;  /*opt-block+*/
                      
                      break;
                    case 5: 
                      goto MATCH_label_l4; break;
                    default: assert(0);
                  } /* (MATCH_w_8_8 & 0x7) -- r_m at 8 --*/ 
                break;
              case 1: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                  unsigned _a = 
                    (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
                  unsigned _b = 1 + addressToPC(MATCH_p);
                  nextPC = 4 + MATCH_p; 
                  
                  #line 276 "machine/pentium/386.pat.m"
                  

                  		if (!a_isVAR && (int)_a != a) return false; else a = _a;

                  		if (!Reg(_b, b, b_isVAR)) return false;

                  		lc = nextPC;

                  		return true;

                  
                  
                  
                } /*opt-block*//*opt-block+*/
                else 
                  goto MATCH_label_l2;  /*opt-block+*/
                
                break;
              case 2: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                  goto MATCH_label_l3;  /*opt-block+*/
                else 
                  goto MATCH_label_l4;  /*opt-block+*/
                
                break;
              case 3: 
                goto MATCH_label_l1; break;
              default: assert(0);
            } /* (MATCH_w_8_8 >> 6 & 0x3) -- mod at 8 --*/ 
          
        } /*opt-block*/
        else 
          goto MATCH_label_l0;  /*opt-block+*/ 
      else 
        goto MATCH_label_l0;  /*opt-block+*/ 
    else 
      goto MATCH_label_l0;  /*opt-block+*/
    
  }goto MATCH_finished_l; 
  
  MATCH_label_l0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 280 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_l; 
    
  MATCH_label_l1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 2 + MATCH_p; 
      
      #line 276 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!Reg(_b, b, b_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_l; 
    
  MATCH_label_l2: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 3 + MATCH_p; 
      
      #line 276 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!Reg(_b, b, b_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_l; 
    
  MATCH_label_l3: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 7 + MATCH_p; 
      
      #line 276 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!Reg(_b, b, b_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_l; 
    
  MATCH_label_l4: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 6 + MATCH_p; 
      
      #line 276 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!Reg(_b, b, b_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_l; 
    
  MATCH_finished_l: (void)0; /*placeholder for label*/
  
}

#line 284 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::POPod(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 286 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 286 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 5 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) { 
      unsigned _a = (MATCH_w_8_0 & 0x7) /* r32 at 0 */;
      nextPC = 1 + MATCH_p; 
      
      #line 288 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_k0;  /*opt-block+*/
    
  }goto MATCH_finished_k; 
  
  MATCH_label_k0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 291 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_k; 
    
  MATCH_finished_k: (void)0; /*placeholder for label*/
  
}

#line 295 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::PUSHod(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 297 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 297 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 5 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0) { 
      unsigned _a = (MATCH_w_8_0 & 0x7) /* r32 at 0 */;
      nextPC = 1 + MATCH_p; 
      
      #line 299 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_j0;  /*opt-block+*/
    
  }goto MATCH_finished_j; 
  
  MATCH_label_j0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 302 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_j; 
    
  MATCH_finished_j: (void)0; /*placeholder for label*/
  
}

#line 306 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::REP$MOVSvod(ADDRESS& lc) {
	ADDRESS nextPC;


#line 308 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 308 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  { 
    MATCH_w_8_8 = getByte(1 + MATCH_p); 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if (0 <= (MATCH_w_8_0 & 0x7) /* col at 0 */ && (MATCH_w_8_0 & 0x7) 
            /* col at 0 */ < 3 || 4 <= (MATCH_w_8_0 & 0x7) /* col at 0 */ && 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ < 8 || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 3 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0 && 
      (0 <= (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ < 15) || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 3 && (MATCH_w_8_0 >> 3 & 0x1) 
            /* page at 0 */ == 0 && (MATCH_w_8_0 >> 4 & 0xf) 
            /* row at 0 */ == 15 && (0 <= (MATCH_w_8_8 & 0x7) 
            /* col at 8 */ && (MATCH_w_8_8 & 0x7) /* col at 8 */ < 5 || 
      6 <= (MATCH_w_8_8 & 0x7) /* col at 8 */ && (MATCH_w_8_8 & 0x7) 
            /* col at 8 */ < 8) || (MATCH_w_8_0 & 0x7) /* col at 0 */ == 3 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 15 && (MATCH_w_8_8 & 0x7) 
            /* col at 8 */ == 5 && (0 <= (MATCH_w_8_8 >> 4 & 0xf) 
            /* row at 8 */ && (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ < 10 || 
      11 <= (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ && 
      (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ < 16) || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 3 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 15 && 
      (MATCH_w_8_8 & 0x7) /* col at 8 */ == 5 && 
      (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ == 10 && 
      (MATCH_w_8_8 >> 3 & 0x1) /* page at 8 */ == 1 || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 3 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) 
      goto MATCH_label_i0;  /*opt-block+*/
    else { 
      nextPC = 2 + MATCH_p; 
      
      #line 310 "machine/pentium/386.pat.m"
      

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_i; 
  
  MATCH_label_i0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 312 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_i; 
    
  MATCH_finished_i: (void)0; /*placeholder for label*/
  
}

#line 316 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::REPNE$SCASB(ADDRESS& lc) {
	ADDRESS nextPC;


#line 318 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 318 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  { 
    MATCH_w_8_8 = getByte(1 + MATCH_p); 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if (0 <= (MATCH_w_8_0 & 0x7) /* col at 0 */ && (MATCH_w_8_0 & 0x7) 
            /* col at 0 */ < 2 || 3 <= (MATCH_w_8_0 & 0x7) /* col at 0 */ && 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ < 8 || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 2 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0 && 
      (0 <= (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ < 15) || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 2 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 15 && 
      (0 <= (MATCH_w_8_8 & 0x7) /* col at 8 */ && (MATCH_w_8_8 & 0x7) 
            /* col at 8 */ < 6 || (MATCH_w_8_8 & 0x7) /* col at 8 */ == 7) || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 2 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 15 && (MATCH_w_8_8 & 0x7) 
            /* col at 8 */ == 6 && (0 <= (MATCH_w_8_8 >> 4 & 0xf) 
            /* row at 8 */ && (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ < 10 || 
      11 <= (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ && 
      (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ < 16) || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 2 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 15 && 
      (MATCH_w_8_8 & 0x7) /* col at 8 */ == 6 && 
      (MATCH_w_8_8 >> 4 & 0xf) /* row at 8 */ == 10 && 
      (MATCH_w_8_8 >> 3 & 0x1) /* page at 8 */ == 0 || 
      (MATCH_w_8_0 & 0x7) /* col at 0 */ == 2 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) 
      goto MATCH_label_h0;  /*opt-block+*/
    else { 
      nextPC = 2 + MATCH_p; 
      
      #line 320 "machine/pentium/386.pat.m"
      

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_h; 
  
  MATCH_label_h0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 322 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_h; 
    
  MATCH_finished_h: (void)0; /*placeholder for label*/
  
}

#line 326 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::RET(ADDRESS& lc) {
	ADDRESS nextPC;


#line 328 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 328 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 3 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 12 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0) { 
      nextPC = 1 + MATCH_p; 
      
      #line 330 "machine/pentium/386.pat.m"
      

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_g0;  /*opt-block+*/
    
  }goto MATCH_finished_g; 
  
  MATCH_label_g0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 332 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_g; 
    
  MATCH_finished_g: (void)0; /*placeholder for label*/
  
}

#line 336 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::RET$Iw(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 338 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 338 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..65535] */ MATCH_w_16_8;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 2 && 
      (MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 12 && 
      (MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 0) { 
      MATCH_w_16_8 = getWord(1 + MATCH_p); 
      { 
        unsigned _a = (MATCH_w_16_8 & 0xffff) /* i16 at 8 */;
        nextPC = 3 + MATCH_p; 
        
        #line 340 "machine/pentium/386.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    else 
      goto MATCH_label_f0;  /*opt-block+*/
    
  }goto MATCH_finished_f; 
  
  MATCH_label_f0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 343 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_f; 
    
  MATCH_finished_f: (void)0; /*placeholder for label*/
  
}

#line 346 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::Reg(ADDRESS& lc, int& a, bool a_isVAR) {


#line 348 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 348 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 >> 6 & 0x3) /* mod at 0 */ == 3) { 
      unsigned _a = (MATCH_w_8_0 & 0x7) /* r_m at 0 */;
      
      #line 350 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      
      #line 352 "machine/pentium/386.pat.m"
      
      		return false;

      
       /*opt-block+*/
    
  }goto MATCH_finished_e; 
  
  MATCH_finished_e: (void)0; /*placeholder for label*/
  
}

#line 356 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::SUBid$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 358 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 358 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  unsigned MATCH_w_32_16;
  unsigned MATCH_w_32_24;
  unsigned MATCH_w_32_32;
  unsigned MATCH_w_32_48;
  unsigned MATCH_w_32_56;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 1) 
      if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 8) 
        if ((MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) 
          goto MATCH_label_d0;  /*opt-block+*/
        else { 
          MATCH_w_8_8 = getByte(1 + MATCH_p); 
          if ((MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */ == 5) 
            
              switch((MATCH_w_8_8 >> 6 & 0x3) /* mod at 8 */) {
                case 0: 
                  
                    switch((MATCH_w_8_8 & 0x7) /* r_m at 8 */) {
                      case 0: case 1: case 2: case 3: case 6: case 7: 
                        MATCH_w_32_16 = getDword(2 + MATCH_p); 
                        goto MATCH_label_d1; 
                        
                        break;
                      case 4: 
                        MATCH_w_8_16 = getByte(2 + MATCH_p); 
                        if ((MATCH_w_8_16 & 0x7) /* base at 16 */ == 5 && 
                          (0 <= (MATCH_w_8_16 >> 3 & 0x7) 
                                /* index at 16 */ && 
                          (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                          MATCH_w_32_56 = getDword(7 + MATCH_p); 
                          goto MATCH_label_d3; 
                          
                        } /*opt-block*/
                        else { 
                          MATCH_w_32_24 = getDword(3 + MATCH_p); 
                          goto MATCH_label_d2; 
                          
                        } /*opt-block*/
                        
                        break;
                      case 5: 
                        MATCH_w_32_48 = getDword(6 + MATCH_p); 
                        goto MATCH_label_d4; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_8_8 & 0x7) -- r_m at 8 --*/ 
                  break;
                case 1: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                    (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                    (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                    MATCH_w_32_32 = getDword(4 + MATCH_p); 
                    { 
                      unsigned _a = 1 + addressToPC(MATCH_p);
                      unsigned _b = MATCH_w_32_32 /* i32 at 32 */;
                      nextPC = 8 + MATCH_p; 
                      
                      #line 360 "machine/pentium/386.pat.m"
                      

                      		if (!Reg(_a, a, a_isVAR)) return false;

                      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

                      		lc = nextPC;

                      		return true;

                      
                      
                      
                    }
                    
                  } /*opt-block*/
                  else { 
                    MATCH_w_32_24 = getDword(3 + MATCH_p); 
                    goto MATCH_label_d2; 
                    
                  } /*opt-block*/
                  
                  break;
                case 2: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                    (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                    (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                    MATCH_w_32_56 = getDword(7 + MATCH_p); 
                    goto MATCH_label_d3; 
                    
                  } /*opt-block*/
                  else { 
                    MATCH_w_32_48 = getDword(6 + MATCH_p); 
                    goto MATCH_label_d4; 
                    
                  } /*opt-block*/
                  
                  break;
                case 3: 
                  MATCH_w_32_16 = getDword(2 + MATCH_p); 
                  goto MATCH_label_d1; 
                  
                  break;
                default: assert(0);
              } /* (MATCH_w_8_8 >> 6 & 0x3) -- mod at 8 --*/  
          else 
            goto MATCH_label_d0;  /*opt-block+*/
          
        } /*opt-block*/ 
      else 
        goto MATCH_label_d0;  /*opt-block+*/ 
    else 
      goto MATCH_label_d0;  /*opt-block+*/
    
  }goto MATCH_finished_d; 
  
  MATCH_label_d0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 364 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = MATCH_w_32_16 /* i32 at 16 */;
      nextPC = 6 + MATCH_p; 
      
      #line 360 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d2: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = MATCH_w_32_24 /* i32 at 24 */;
      nextPC = 7 + MATCH_p; 
      
      #line 360 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d3: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = MATCH_w_32_56 /* i32 at 56 */;
      nextPC = 11 + MATCH_p; 
      
      #line 360 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_label_d4: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = MATCH_w_32_48 /* i32 at 48 */;
      nextPC = 10 + MATCH_p; 
      
      #line 360 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_finished_d: (void)0; /*placeholder for label*/
  
}

#line 368 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::SUBiodb$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 370 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 370 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  unsigned /* [0..255] */ MATCH_w_8_24;
  unsigned /* [0..255] */ MATCH_w_8_32;
  unsigned /* [0..255] */ MATCH_w_8_48;
  unsigned /* [0..255] */ MATCH_w_8_56;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 3) 
      if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 8) 
        if ((MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) 
          goto MATCH_label_c0;  /*opt-block+*/
        else { 
          MATCH_w_8_8 = getByte(1 + MATCH_p); 
          if ((MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */ == 5) 
            
              switch((MATCH_w_8_8 >> 6 & 0x3) /* mod at 8 */) {
                case 0: 
                  
                    switch((MATCH_w_8_8 & 0x7) /* r_m at 8 */) {
                      case 0: case 1: case 2: case 3: case 6: case 7: 
                        MATCH_w_8_16 = getByte(2 + MATCH_p); 
                        goto MATCH_label_c1; 
                        
                        break;
                      case 4: 
                        MATCH_w_8_16 = getByte(2 + MATCH_p); 
                        if ((MATCH_w_8_16 & 0x7) /* base at 16 */ == 5 && 
                          (0 <= (MATCH_w_8_16 >> 3 & 0x7) 
                                /* index at 16 */ && 
                          (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                          MATCH_w_8_56 = getByte(7 + MATCH_p); 
                          goto MATCH_label_c3; 
                          
                        } /*opt-block*/
                        else { 
                          MATCH_w_8_24 = getByte(3 + MATCH_p); 
                          goto MATCH_label_c2; 
                          
                        } /*opt-block*/
                        
                        break;
                      case 5: 
                        MATCH_w_8_48 = getByte(6 + MATCH_p); 
                        goto MATCH_label_c4; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_8_8 & 0x7) -- r_m at 8 --*/ 
                  break;
                case 1: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                    (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                    (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                    MATCH_w_8_32 = getByte(4 + MATCH_p); 
                    { 
                      unsigned _a = 1 + addressToPC(MATCH_p);
                      int /* [~128..127] */ _b = 
                        sign_extend((MATCH_w_8_32 & 0xff) /* i8 at 32 */, 8);
                      nextPC = 5 + MATCH_p; 
                      
                      #line 372 "machine/pentium/386.pat.m"
                      

                      		if (!Reg(_a, a, a_isVAR)) return false;

                      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

                      		lc = nextPC;

                      		return true;

                      
                      
                      
                    }
                    
                  } /*opt-block*/
                  else { 
                    MATCH_w_8_24 = getByte(3 + MATCH_p); 
                    goto MATCH_label_c2; 
                    
                  } /*opt-block*/
                  
                  break;
                case 2: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                    (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                    (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                    MATCH_w_8_56 = getByte(7 + MATCH_p); 
                    goto MATCH_label_c3; 
                    
                  } /*opt-block*/
                  else { 
                    MATCH_w_8_48 = getByte(6 + MATCH_p); 
                    goto MATCH_label_c4; 
                    
                  } /*opt-block*/
                  
                  break;
                case 3: 
                  MATCH_w_8_16 = getByte(2 + MATCH_p); 
                  goto MATCH_label_c1; 
                  
                  break;
                default: assert(0);
              } /* (MATCH_w_8_8 >> 6 & 0x3) -- mod at 8 --*/  
          else 
            goto MATCH_label_c0;  /*opt-block+*/
          
        } /*opt-block*/ 
      else 
        goto MATCH_label_c0;  /*opt-block+*/ 
    else 
      goto MATCH_label_c0;  /*opt-block+*/
    
  }goto MATCH_finished_c; 
  
  MATCH_label_c0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 376 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_c; 
    
  MATCH_label_c1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_16 & 0xff) /* i8 at 16 */, 8);
      nextPC = 3 + MATCH_p; 
      
      #line 372 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_c; 
    
  MATCH_label_c2: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_24 & 0xff) /* i8 at 24 */, 8);
      nextPC = 4 + MATCH_p; 
      
      #line 372 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_c; 
    
  MATCH_label_c3: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_56 & 0xff) /* i8 at 56 */, 8);
      nextPC = 8 + MATCH_p; 
      
      #line 372 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_c; 
    
  MATCH_label_c4: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      int /* [~128..127] */ _b = 
        sign_extend((MATCH_w_8_48 & 0xff) /* i8 at 48 */, 8);
      nextPC = 7 + MATCH_p; 
      
      #line 372 "machine/pentium/386.pat.m"
      

      		if (!Reg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_c; 
    
  MATCH_finished_c: (void)0; /*placeholder for label*/
  
}

#line 380 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::XCHG$Ev$Gvod$E$Base(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 382 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 382 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 8) 
      if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 7) 
        if ((MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) 
          goto MATCH_label_b0;  /*opt-block+*/
        else { 
          MATCH_w_8_8 = getByte(1 + MATCH_p); 
          
            switch((MATCH_w_8_8 >> 6 & 0x3) /* mod at 8 */) {
              case 0: 
                
                  switch((MATCH_w_8_8 & 0x7) /* r_m at 8 */) {
                    case 0: case 1: case 2: case 3: case 6: case 7: 
                      goto MATCH_label_b1; break;
                    case 4: 
                      MATCH_w_8_16 = getByte(2 + MATCH_p); 
                      if ((MATCH_w_8_16 & 0x7) /* base at 16 */ == 5 && 
                        (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                        (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                        goto MATCH_label_b3;  /*opt-block+*/
                      else 
                        goto MATCH_label_b2;  /*opt-block+*/
                      
                      break;
                    case 5: 
                      goto MATCH_label_b4; break;
                    default: assert(0);
                  } /* (MATCH_w_8_8 & 0x7) -- r_m at 8 --*/ 
                break;
              case 1: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                  unsigned _a = 1 + addressToPC(MATCH_p);
                  unsigned _b = 
                    (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
                  nextPC = 4 + MATCH_p; 
                  
                  #line 384 "machine/pentium/386.pat.m"
                  

                  		if (!E$Base(_a, a, a_isVAR)) return false;

                  		if (!b_isVAR && (int)_b != b) return false; else b = _b;

                  		lc = nextPC;

                  		return true;

                  
                  
                  
                } /*opt-block*//*opt-block+*/
                else 
                  goto MATCH_label_b2;  /*opt-block+*/
                
                break;
              case 2: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                  goto MATCH_label_b3;  /*opt-block+*/
                else 
                  goto MATCH_label_b4;  /*opt-block+*/
                
                break;
              case 3: 
                goto MATCH_label_b1; break;
              default: assert(0);
            } /* (MATCH_w_8_8 >> 6 & 0x3) -- mod at 8 --*/ 
          
        } /*opt-block*/ 
      else 
        goto MATCH_label_b0;  /*opt-block+*/ 
    else 
      goto MATCH_label_b0;  /*opt-block+*/
    
  }goto MATCH_finished_b; 
  
  MATCH_label_b0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 388 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_b; 
    
  MATCH_label_b1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      nextPC = 2 + MATCH_p; 
      
      #line 384 "machine/pentium/386.pat.m"
      

      		if (!E$Base(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_b; 
    
  MATCH_label_b2: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      nextPC = 3 + MATCH_p; 
      
      #line 384 "machine/pentium/386.pat.m"
      

      		if (!E$Base(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_b; 
    
  MATCH_label_b3: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      nextPC = 7 + MATCH_p; 
      
      #line 384 "machine/pentium/386.pat.m"
      

      		if (!E$Base(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_b; 
    
  MATCH_label_b4: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 1 + addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      nextPC = 6 + MATCH_p; 
      
      #line 384 "machine/pentium/386.pat.m"
      

      		if (!E$Base(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_b; 
    
  MATCH_finished_b: (void)0; /*placeholder for label*/
  
}

#line 392 "machine/pentium/386.pat.m"
}
bool InstructionPatterns::XORrmod$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 394 "machine/pentium/386.pat.m"
{ 
  dword MATCH_p = 
    
    #line 394 "machine/pentium/386.pat.m"
    lc
    ;
  unsigned /* [0..255] */ MATCH_w_8_0;
  unsigned /* [0..255] */ MATCH_w_8_8;
  unsigned /* [0..255] */ MATCH_w_8_16;
  { 
    MATCH_w_8_0 = getByte(MATCH_p); 
    if ((MATCH_w_8_0 & 0x7) /* col at 0 */ == 3) 
      if ((MATCH_w_8_0 >> 4 & 0xf) /* row at 0 */ == 3) 
        if ((MATCH_w_8_0 >> 3 & 0x1) /* page at 0 */ == 1) 
          goto MATCH_label_a0;  /*opt-block+*/
        else { 
          MATCH_w_8_8 = getByte(1 + MATCH_p); 
          
            switch((MATCH_w_8_8 >> 6 & 0x3) /* mod at 8 */) {
              case 0: 
                
                  switch((MATCH_w_8_8 & 0x7) /* r_m at 8 */) {
                    case 0: case 1: case 2: case 3: case 6: case 7: 
                      goto MATCH_label_a1; break;
                    case 4: 
                      MATCH_w_8_16 = getByte(2 + MATCH_p); 
                      if ((MATCH_w_8_16 & 0x7) /* base at 16 */ == 5 && 
                        (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                        (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                        goto MATCH_label_a3;  /*opt-block+*/
                      else 
                        goto MATCH_label_a2;  /*opt-block+*/
                      
                      break;
                    case 5: 
                      goto MATCH_label_a4; break;
                    default: assert(0);
                  } /* (MATCH_w_8_8 & 0x7) -- r_m at 8 --*/ 
                break;
              case 1: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) { 
                  unsigned _a = 
                    (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
                  unsigned _b = 1 + addressToPC(MATCH_p);
                  nextPC = 4 + MATCH_p; 
                  
                  #line 396 "machine/pentium/386.pat.m"
                  

                  		if (!a_isVAR && (int)_a != a) return false; else a = _a;

                  		if (!Reg(_b, b, b_isVAR)) return false;

                  		lc = nextPC;

                  		return true;

                  
                  
                  
                } /*opt-block*//*opt-block+*/
                else 
                  goto MATCH_label_a2;  /*opt-block+*/
                
                break;
              case 2: 
                MATCH_w_8_16 = getByte(2 + MATCH_p); 
                if ((MATCH_w_8_8 & 0x7) /* r_m at 8 */ == 4 && 
                  (0 <= (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ && 
                  (MATCH_w_8_16 >> 3 & 0x7) /* index at 16 */ < 8)) 
                  goto MATCH_label_a3;  /*opt-block+*/
                else 
                  goto MATCH_label_a4;  /*opt-block+*/
                
                break;
              case 3: 
                goto MATCH_label_a1; break;
              default: assert(0);
            } /* (MATCH_w_8_8 >> 6 & 0x3) -- mod at 8 --*/ 
          
        } /*opt-block*/ 
      else 
        goto MATCH_label_a0;  /*opt-block+*/ 
    else 
      goto MATCH_label_a0;  /*opt-block+*/
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 400 "machine/pentium/386.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 2 + MATCH_p; 
      
      #line 396 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!Reg(_b, b, b_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a2: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 3 + MATCH_p; 
      
      #line 396 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!Reg(_b, b, b_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a3: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 7 + MATCH_p; 
      
      #line 396 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!Reg(_b, b, b_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a4: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_8_8 >> 3 & 0x7) /* reg_opcode at 8 */;
      unsigned _b = 1 + addressToPC(MATCH_p);
      nextPC = 6 + MATCH_p; 
      
      #line 396 "machine/pentium/386.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!Reg(_b, b, b_isVAR)) return false;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 404 "machine/pentium/386.pat.m"
}
Logue* InstructionPatterns::std_call(CSR& csr, ADDRESS& lc, int& addr)
{
	ADDRESS __save = lc;
	if (
	CALL$Jvod(lc, addr, VAR)) {
		vector<int> params(1); params[0] = addr; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("std_call",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::pat_strlen(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	int __loc0;
	if (
	XORrmod$Reg(lc, EAX, VAL, EAX, VAL) && 
	CLD(lc) && 
	MOVid(lc, ECX, VAL, __loc0 = -1, VAL) && 
	REPNE$SCASB(lc)) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("pat_strlen",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::pat_memcpy_00(CSR& csr, ADDRESS& lc, int& len)
{
	ADDRESS __save = lc;
	if (
	CLD(lc) && 
	MOVid(lc, ECX, VAL, len, VAR) && 
	REP$MOVSvod(lc)) {
		vector<int> params(1); params[0] = len; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("pat_memcpy_00",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::pat_memcpy_01(CSR& csr, ADDRESS& lc, int& len)
{
	ADDRESS __save = lc;
	if (
	CLD(lc) && 
	MOVid(lc, ECX, VAL, len, VAR) && 
	REP$MOVSvod(lc) && 
	MOVSB(lc)) {
		vector<int> params(1); params[0] = len; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("pat_memcpy_01",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::pat_memcpy_10(CSR& csr, ADDRESS& lc, int& len)
{
	ADDRESS __save = lc;
	if (
	CLD(lc) && 
	MOVid(lc, ECX, VAL, len, VAR) && 
	REP$MOVSvod(lc) && 
	MOVSvow(lc)) {
		vector<int> params(1); params[0] = len; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("pat_memcpy_10",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::pat_memcpy_11(CSR& csr, ADDRESS& lc, int& len)
{
	ADDRESS __save = lc;
	if (
	CLD(lc) && 
	MOVid(lc, ECX, VAL, len, VAR) && 
	REP$MOVSvod(lc) && 
	MOVSvow(lc) && 
	MOVSB(lc)) {
		vector<int> params(1); params[0] = len; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("pat_memcpy_11",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::pat_this_thunk(CSR& csr, ADDRESS& lc, int& off, int& dest)
{
	ADDRESS __save = lc;
	int __loc0;
	if (
	ADDiodb$E$Base8(lc, __loc0 = 4, VAL, ESP, VAL, off, VAR) && 
	JMP$Jvod(lc, dest, VAR)) {
		vector<int> params(2); params[0] = off; params[1] = dest; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("pat_this_thunk",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::std_entry(CSR& csr, ADDRESS& lc, int& locals, int& regs)
{
	ADDRESS __save = lc;
	locals = 0;
	if (
	PUSHod(lc, EBP, VAL) && 
	((MOVrmod$Reg(lc, EBP, VAL, ESP, VAL) || 
	MOVmrod$Reg(lc, EBP, VAL, ESP, VAL)) || true) && 
	((SUBiodb$Reg(lc, ESP, VAL, locals, VAR) || 
	SUBid$Reg(lc, ESP, VAL, locals, VAR)) || true) && 
((iterhlp0(lc,regs)) || true)) {
		vector<int> params(2); params[0] = locals; params[1] = regs; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("std_entry",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::struct_ptr(CSR& csr, ADDRESS& lc, int& locals, int& regs)
{
	ADDRESS __save = lc;
	if (
	POPod(lc, EAX, VAL) && 
	XCHG$Ev$Gvod$E$Base(lc, ESP, VAL, EAX, VAL) && 
	std_entry(csr, lc, locals, regs)) {
		vector<int> params(2); params[0] = locals; params[1] = regs; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("struct_ptr",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::frameless_pro(CSR& csr, ADDRESS& lc, int& locals, int& regs)
{
	ADDRESS __save = lc;
	if (
	((SUBiodb$Reg(lc, ESP, VAL, locals, VAR) || 
	SUBid$Reg(lc, ESP, VAL, locals, VAR)) || true) && 
((iterhlp1(lc,regs)) || true)) {
		vector<int> params(2); params[0] = locals; params[1] = regs; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("frameless_pro",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::none(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	if (
	(lc += 0, true)) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("none",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::std_ret(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	int __loop;
	int __loc0;
	if (
	((LEAod$Disp8(lc, ESP, VAL, __loc0, VAR, EBP, VAL)) || true) && 
((iterhlp2(lc,__loop,__loc0) || 
iterhlp3(lc,__loop,__loc0)) || true) && 
	(LEAVE(lc) || 
	(MOVrmod$Reg(lc, ESP, VAL, EBP, VAL) && 
	POPod(lc, EBP, VAL))) && 
	(RET(lc) || 
	RET$Iw(lc, __loc0, VAR))) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("std_ret",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::frameless_epi(CSR& csr, ADDRESS& lc, int& n)
{
	ADDRESS __save = lc;
	int __loop;
	int __loc0;
	if (
((iterhlp4(lc,__loop)) || true) && 
	(((ADDiodb$Reg(lc, ESP, VAL, n, VAR) || 
	ADDid$Reg(lc, ESP, VAL, n, VAR))) || true) && 
	(RET(lc) || 
	RET$Iw(lc, __loc0, VAR))) {
		vector<int> params(1); params[0] = n; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("frameless_epi",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::clear_stack(CSR& csr, ADDRESS& lc, int& n)
{
	ADDRESS __save = lc;
	if (
	(ADDiodb$Reg(lc, ESP, VAL, n, VAR) || 
	ADDid$Reg(lc, ESP, VAL, n, VAR)) || 
iterhlp5(lc,n)) {
		vector<int> params(1); params[0] = n; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("clear_stack",params);
	} else {
		lc = __save;
		return NULL;
	}
}
LogueDict::LogueDict()
{
	{
		list<string> params;
		params.push_back("addr");
		theSemTable.addItem("addr");
		this->newLogue("std_call","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("pat_strlen","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("len");
		theSemTable.addItem("len");
		this->newLogue("pat_memcpy_00","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("len");
		theSemTable.addItem("len");
		this->newLogue("pat_memcpy_01","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("len");
		theSemTable.addItem("len");
		this->newLogue("pat_memcpy_10","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("len");
		theSemTable.addItem("len");
		this->newLogue("pat_memcpy_11","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("off");
		theSemTable.addItem("off");
		params.push_back("dest");
		theSemTable.addItem("dest");
		this->newLogue("pat_this_thunk","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("locals");
		theSemTable.addItem("locals");
		params.push_back("regs");
		theSemTable.addItem("regs");
		this->newLogue("std_entry","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("locals");
		theSemTable.addItem("locals");
		params.push_back("regs");
		theSemTable.addItem("regs");
		this->newLogue("struct_ptr","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("locals");
		theSemTable.addItem("locals");
		params.push_back("regs");
		theSemTable.addItem("regs");
		this->newLogue("frameless_pro","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("none","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("std_ret","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		params.push_back("n");
		theSemTable.addItem("n");
		this->newLogue("frameless_epi","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		params.push_back("n");
		theSemTable.addItem("n");
		this->newLogue("clear_stack","CALLER_EPILOGUE",params);
	}
}


