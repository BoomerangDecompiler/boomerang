#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 2 "machine/sparc/sparc.pat.m"
/*==============================================
 * FILE:      sparc.pat.m
 * OVERVIEW:  Generated file; do not edit
 *
 * (C) 1998-2000 The University of Queensland, BT group
 *==============================================*/

#include "global.h"
#include "decoder.h"
#include "sparc.pat.h"
#include "ss.h"
#include "csr.h"

#define VAR true
#define VAL false
int InstructionPatterns::SP = 14;
int InstructionPatterns::FP = 30;
int InstructionPatterns::o0 = 8;
int InstructionPatterns::i0 = 24;
int InstructionPatterns::i7 = 31;
int InstructionPatterns::o7 = 15;
int InstructionPatterns::g0 = 0;
bool InstructionPatterns::ADD$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 25 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 25 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ && 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ < 2 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 3 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (1 <= (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ < 64)) 
      goto MATCH_label_u0;  /*opt-block+*/
    else { 
      unsigned _a = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 27 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!imode(_b, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_c != c) return false; else c = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_u; 
  
  MATCH_label_u0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 32 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_u; 
    
  MATCH_finished_u: (void)0; /*placeholder for label*/
  
}

#line 36 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::ADD$rmode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 38 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 38 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ && 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ < 2 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 3 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (1 <= (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ < 64)) 
      goto MATCH_label_t0;  /*opt-block+*/
    else { 
      unsigned _a = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 40 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!rmode(_b, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_c != c) return false; else c = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_t; 
  
  MATCH_label_t0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 45 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_t; 
    
  MATCH_finished_t: (void)0; /*placeholder for label*/
  
}

#line 49 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::JMPL$dispA(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 51 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 51 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 56 && 
      (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2)) { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 53 "machine/sparc/sparc.pat.m"
      

      		if (!dispA(_a, a, a_isVAR, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_b != c) return false; else c = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_s0;  /*opt-block+*/
    
  }goto MATCH_finished_s; 
  
  MATCH_label_s0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 57 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_s; 
    
  MATCH_finished_s: (void)0; /*placeholder for label*/
  
}

#line 61 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::JMPL$indirectA(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 63 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 63 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 56 && 
      (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2)) { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 65 "machine/sparc/sparc.pat.m"
      

      		if (!indirectA(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_r0;  /*opt-block+*/
    
  }goto MATCH_finished_r; 
  
  MATCH_label_r0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 69 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_r; 
    
  MATCH_finished_r: (void)0; /*placeholder for label*/
  
}

#line 73 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::OR$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 75 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 75 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 2 && 
      (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2)) { 
      unsigned _a = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 77 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!imode(_b, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_c != c) return false; else c = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_q0;  /*opt-block+*/
    
  }goto MATCH_finished_q; 
  
  MATCH_label_q0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 82 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_q; 
    
  MATCH_finished_q: (void)0; /*placeholder for label*/
  
}

#line 86 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::RESTORE$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 88 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 88 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 61 && 
      (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2)) { 
      unsigned _a = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 90 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!imode(_b, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_c != c) return false; else c = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_p0;  /*opt-block+*/
    
  }goto MATCH_finished_p; 
  
  MATCH_label_p0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 95 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_p; 
    
  MATCH_finished_p: (void)0; /*placeholder for label*/
  
}

#line 99 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::RESTORE$rmode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 101 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 101 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 61 && 
      (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2)) { 
      unsigned _a = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 103 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!rmode(_b, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_c != c) return false; else c = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_o0;  /*opt-block+*/
    
  }goto MATCH_finished_o; 
  
  MATCH_label_o0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 108 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_o; 
    
  MATCH_finished_o: (void)0; /*placeholder for label*/
  
}

#line 112 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::SAVE$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 114 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 114 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 60 && 
      (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2)) { 
      unsigned _a = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 116 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!imode(_b, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_c != c) return false; else c = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_n0;  /*opt-block+*/
    
  }goto MATCH_finished_n; 
  
  MATCH_label_n0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 121 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_n; 
    
  MATCH_finished_n: (void)0; /*placeholder for label*/
  
}

#line 125 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::SAVE$rmode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 127 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 127 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 60 && 
      (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2)) { 
      unsigned _a = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 129 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!rmode(_b, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_c != c) return false; else c = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_m0;  /*opt-block+*/
    
  }goto MATCH_finished_m; 
  
  MATCH_label_m0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 134 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_m; 
    
  MATCH_finished_m: (void)0; /*placeholder for label*/
  
}

#line 138 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::SUB$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 140 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 140 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 4 && 
      (0 <= (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2)) { 
      unsigned _a = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 142 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!imode(_b, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_c != c) return false; else c = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_l0;  /*opt-block+*/
    
  }goto MATCH_finished_l; 
  
  MATCH_label_l0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 147 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_l; 
    
  MATCH_finished_l: (void)0; /*placeholder for label*/
  
}

#line 151 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::UNIMP(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 153 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 153 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 0 && 
      (MATCH_w_32_0 >> 22 & 0x7) /* op2 at 0 */ == 0) { 
      unsigned _a = (MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 155 "machine/sparc/sparc.pat.m"
      

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
      
      #line 158 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_k; 
    
  MATCH_finished_k: (void)0; /*placeholder for label*/
  
}

#line 162 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::call__(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 164 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 164 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 1) { 
      unsigned _a = 
        4 * sign_extend((MATCH_w_32_0 & 0x3fffffff) /* disp30 at 0 */, 30) + 
        addressToPC(MATCH_p);
      nextPC = 4 + MATCH_p; 
      
      #line 166 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else { 
      nextPC = MATCH_p; 
      
      #line 169 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_j; 
  
  MATCH_finished_j: (void)0; /*placeholder for label*/
  
}

#line 172 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::dispA(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {


#line 174 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 174 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
      unsigned _a = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      int /* [~4096..4095] */ _b = 
        sign_extend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
      
      #line 176 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      
      #line 179 "machine/sparc/sparc.pat.m"
      
      		return false;

      
       /*opt-block+*/
    
  }goto MATCH_finished_i; 
  
  MATCH_finished_i: (void)0; /*placeholder for label*/
  
}

#line 182 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::imode(ADDRESS& lc, int& a, bool a_isVAR) {


#line 184 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 184 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) { 
      int /* [~4096..4095] */ _a = 
        sign_extend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
      
      #line 186 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      
      #line 188 "machine/sparc/sparc.pat.m"
      
      		return false;

      
       /*opt-block+*/
    
  }goto MATCH_finished_h; 
  
  MATCH_finished_h: (void)0; /*placeholder for label*/
  
}

#line 191 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::indirectA(ADDRESS& lc, int& a, bool a_isVAR) {


#line 193 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 193 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0 && 
      (1 <= (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */ && 
      (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */ < 32) || 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) 
      goto MATCH_label_g0;  /*opt-block+*/
    else { 
      unsigned _a = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
      
      #line 195 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_g; 
  
  MATCH_label_g0: (void)0; /*placeholder for label*/ 
    
    #line 197 "machine/sparc/sparc.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_g; 
    
  MATCH_finished_g: (void)0; /*placeholder for label*/
  
}

#line 201 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::mov_(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 203 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 203 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 2 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0 && 
      (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0) { 
      unsigned _a = (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */;
      unsigned _b = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 205 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_f0;  /*opt-block+*/
    
  }goto MATCH_finished_f; 
  
  MATCH_label_f0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 209 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_f; 
    
  MATCH_finished_f: (void)0; /*placeholder for label*/
  
}

#line 213 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::restore_(ADDRESS& lc) {
	ADDRESS nextPC;


#line 215 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 215 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ && 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ < 2 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 3 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (0 <= (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ < 61 || 
      62 <= (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ < 64) || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 61 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0 && 
      (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0 && 
      (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0 && 
      (1 <= (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */ && 
      (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */ < 32) || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 61 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0 && 
      (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0 && 
      (1 <= (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ && 
      (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ < 32) || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 61 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0 && 
      (1 <= (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ && 
      (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ < 32) || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 61 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) 
      goto MATCH_label_e0;  /*opt-block+*/
    else { 
      nextPC = 4 + MATCH_p; 
      
      #line 217 "machine/sparc/sparc.pat.m"
      

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_e; 
  
  MATCH_label_e0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 219 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_e; 
    
  MATCH_finished_e: (void)0; /*placeholder for label*/
  
}

#line 223 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::ret(ADDRESS& lc) {
	ADDRESS nextPC;


#line 225 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 225 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ && 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ < 2 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 3 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (0 <= (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ < 56 || 
      57 <= (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ < 64) || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 56 && 
      (0 <= (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ && 
      (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ < 8 || 
      9 <= (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ && 
      (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ < 8192) || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 56 && 
      (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ == 8 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 56 && 
      (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ == 8 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1 && 
      (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0 && 
      (0 <= (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ && 
      (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ < 31) || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 56 && 
      (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ == 8 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1 && 
      (1 <= (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ && 
      (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ < 32)) 
      goto MATCH_label_d0;  /*opt-block+*/
    else { 
      nextPC = 4 + MATCH_p; 
      
      #line 227 "machine/sparc/sparc.pat.m"
      

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_d; 
  
  MATCH_label_d0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 229 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_finished_d: (void)0; /*placeholder for label*/
  
}

#line 233 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::retl(ADDRESS& lc) {
	ADDRESS nextPC;


#line 235 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 235 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ && 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ < 2 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 3 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (0 <= (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ < 56 || 
      57 <= (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ < 64) || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 56 && 
      (0 <= (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ && 
      (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ < 15 || 
      16 <= (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ && 
      (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ < 32) || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 56 && 
      (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 15 && 
      (0 <= (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ && 
      (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ < 8 || 
      9 <= (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ && 
      (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ < 8192) || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 56 && 
      (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 15 && 
      (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ == 8 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0 || 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2 && 
      (MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 56 && 
      (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 15 && 
      (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */ == 8 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1 && 
      (1 <= (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ && 
      (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ < 32)) 
      goto MATCH_label_c0;  /*opt-block+*/
    else { 
      nextPC = 4 + MATCH_p; 
      
      #line 237 "machine/sparc/sparc.pat.m"
      

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_c; 
  
  MATCH_label_c0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 239 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_c; 
    
  MATCH_finished_c: (void)0; /*placeholder for label*/
  
}

#line 242 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::rmode(ADDRESS& lc, int& a, bool a_isVAR) {


#line 244 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 244 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) 
      
      #line 248 "machine/sparc/sparc.pat.m"
      
      		return false;

      
       /*opt-block+*/
    else { 
      unsigned _a = (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */;
      
      #line 246 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_b; 
  
  MATCH_finished_b: (void)0; /*placeholder for label*/
  
}

#line 252 "machine/sparc/sparc.pat.m"
}
bool InstructionPatterns::sethi(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 254 "machine/sparc/sparc.pat.m"
{ 
  dword MATCH_p = 
    
    #line 254 "machine/sparc/sparc.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 22 & 0x7) /* op2 at 0 */ == 4 && 
      (MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 0) { 
      unsigned _a = (MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */ << 10;
      unsigned _b = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 256 "machine/sparc/sparc.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_a0;  /*opt-block+*/
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 260 "machine/sparc/sparc.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 264 "machine/sparc/sparc.pat.m"
}
Logue* InstructionPatterns::std_call(CSR& csr, ADDRESS& lc, int& addr)
{
	ADDRESS __save = lc;
	if (
	call__(lc, addr, VAR)) {
		vector<int> params(1); params[0] = addr; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("std_call",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::struct_call(CSR& csr, ADDRESS& lc, int& addr, int& imm22)
{
	ADDRESS __save = lc;
	if (
	call__(lc, addr, VAR) && 
	(lc += 4, true) && 
	UNIMP(lc, imm22, VAR)) {
		vector<int> params(2); params[0] = addr; params[1] = imm22; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("struct_call",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::call_rst_ui_reg(CSR& csr, ADDRESS& lc, int& addr, int& imm22, int& rs1, int& rs2, int& rd)
{
	ADDRESS __save = lc;
	if (
	call__(lc, addr, VAR) && 
	RESTORE$rmode(lc, rs1, VAR, rs2, VAR, rd, VAR) && 
	UNIMP(lc, imm22, VAR)) {
		vector<int> params(5); params[0] = addr; params[1] = imm22; params[2] = rs1; params[3] = rs2; params[4] = rd; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("call_rst_ui_reg",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::call_rst_ui_imm(CSR& csr, ADDRESS& lc, int& addr, int& imm22, int& rs1, int& imm, int& rd)
{
	ADDRESS __save = lc;
	if (
	call__(lc, addr, VAR) && 
	RESTORE$imode(lc, rs1, VAR, imm, VAR, rd, VAR) && 
	UNIMP(lc, imm22, VAR)) {
		vector<int> params(5); params[0] = addr; params[1] = imm22; params[2] = rs1; params[3] = imm; params[4] = rd; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("call_rst_ui_imm",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::call_restore_reg(CSR& csr, ADDRESS& lc, int& addr, int& rs1, int& rs2, int& rd)
{
	ADDRESS __save = lc;
	if (
	call__(lc, addr, VAR) && 
	RESTORE$rmode(lc, rs1, VAR, rs2, VAR, rd, VAR)) {
		vector<int> params(4); params[0] = addr; params[1] = rs1; params[2] = rs2; params[3] = rd; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("call_restore_reg",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::call_restore_imm(CSR& csr, ADDRESS& lc, int& addr, int& rs1, int& imm, int& rd)
{
	ADDRESS __save = lc;
	if (
	call__(lc, addr, VAR) && 
	RESTORE$imode(lc, rs1, VAR, imm, VAR, rd, VAR)) {
		vector<int> params(4); params[0] = addr; params[1] = rs1; params[2] = imm; params[3] = rd; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("call_restore_imm",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::move_call_move(CSR& csr, ADDRESS& lc, int& addr, int& rd)
{
	ADDRESS __save = lc;
	if (
	mov_(lc, o7, VAL, rd, VAR) && 
	call__(lc, addr, VAR) && 
	mov_(lc, rd, VAR, o7, VAL)) {
		vector<int> params(2); params[0] = addr; params[1] = rd; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("move_call_move",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::move_x_call_move(CSR& csr, ADDRESS& lc, int& addr, int& rd)
{
	ADDRESS __save = lc;
	if (
	mov_(lc, o7, VAL, rd, VAR) && 
	(lc += 4, true) && 
	call__(lc, addr, VAR) && 
	mov_(lc, rd, VAR, o7, VAL)) {
		vector<int> params(2); params[0] = addr; params[1] = rd; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("move_x_call_move",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::call_add(CSR& csr, ADDRESS& lc, int& addr, int& imm)
{
	ADDRESS __save = lc;
	if (
	call__(lc, addr, VAR) && 
	ADD$imode(lc, o7, VAL, imm, VAR, o7, VAL)) {
		vector<int> params(2); params[0] = addr; params[1] = imm; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("call_add",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::jmp_restore_reg(CSR& csr, ADDRESS& lc, int& rs1j, int& rdj, int& rs1, int& rs2, int& rd)
{
	ADDRESS __save = lc;
	if (
	JMPL$indirectA(lc, rs1j, VAR, rdj, VAR) && 
	RESTORE$rmode(lc, rs1, VAR, rs2, VAR, rd, VAR)) {
		vector<int> params(5); params[0] = rs1j; params[1] = rdj; params[2] = rs1; params[3] = rs2; params[4] = rd; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("jmp_restore_reg",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::jmp_restore_imm(CSR& csr, ADDRESS& lc, int& rs1j, int& rdj, int& rs1, int& imm, int& rd)
{
	ADDRESS __save = lc;
	if (
	JMPL$indirectA(lc, rs1j, VAR, rdj, VAR) && 
	RESTORE$imode(lc, rs1, VAR, imm, VAR, rd, VAR)) {
		vector<int> params(5); params[0] = rs1j; params[1] = rdj; params[2] = rs1; params[3] = imm; params[4] = rd; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("jmp_restore_imm",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::new_reg_win(CSR& csr, ADDRESS& lc, int& locals)
{
	ADDRESS __save = lc;
	if (
	SAVE$imode(lc, SP, VAL, locals, VAR, SP, VAL)) {
		vector<int> params(1); params[0] = locals; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("new_reg_win",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::same_reg_win(CSR& csr, ADDRESS& lc, int& locals)
{
	ADDRESS __save = lc;
	if (
	ADD$imode(lc, SP, VAL, locals, VAR, SP, VAL)) {
		vector<int> params(1); params[0] = locals; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("same_reg_win",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::new_reg_win_large(CSR& csr, ADDRESS& lc, int& hiVal, int& loVal, int& reg)
{
	ADDRESS __save = lc;
	if (
	sethi(lc, hiVal, VAR, reg, VAR) && 
	(ADD$imode(lc, reg, VAR, loVal, VAR, reg, VAR) || 
	OR$imode(lc, reg, VAR, loVal, VAR, reg, VAR)) && 
	SAVE$rmode(lc, SP, VAL, reg, VAR, SP, VAL)) {
		vector<int> params(3); params[0] = hiVal; params[1] = loVal; params[2] = reg; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("new_reg_win_large",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::same_reg_win_large(CSR& csr, ADDRESS& lc, int& hiVal, int& loVal, int& reg)
{
	ADDRESS __save = lc;
	if (
	sethi(lc, hiVal, VAR, reg, VAR) && 
	(ADD$imode(lc, reg, VAR, loVal, VAR, reg, VAR) || 
	OR$imode(lc, reg, VAR, loVal, VAR, reg, VAR)) && 
	ADD$rmode(lc, SP, VAL, reg, VAR, SP, VAL)) {
		vector<int> params(3); params[0] = hiVal; params[1] = loVal; params[2] = reg; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("same_reg_win_large",params);
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
	int __loc0;
	if (
	(ret(lc) || 
	JMPL$dispA(lc, i7, VAL, __loc0 = 12, VAL, g0, VAL)) && 
	restore_(lc)) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("std_ret",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::ret_reg_val(CSR& csr, ADDRESS& lc, int& rs1, int& rs2)
{
	ADDRESS __save = lc;
	int __loc0;
	if (
	(ret(lc) || 
	JMPL$dispA(lc, i7, VAL, __loc0 = 12, VAL, g0, VAL)) && 
	RESTORE$rmode(lc, rs1, VAR, rs2, VAR, o0, VAL)) {
		vector<int> params(2); params[0] = rs1; params[1] = rs2; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("ret_reg_val",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::ret_imm_val(CSR& csr, ADDRESS& lc, int& rs1, int& imm)
{
	ADDRESS __save = lc;
	int __loc0;
	if (
	(ret(lc) || 
	JMPL$dispA(lc, i7, VAL, __loc0 = 12, VAL, g0, VAL)) && 
	RESTORE$imode(lc, rs1, VAR, imm, VAR, o0, VAL)) {
		vector<int> params(2); params[0] = rs1; params[1] = imm; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("ret_imm_val",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::leaf_ret(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	int __loc0;
	if (
	(retl(lc) || 
	JMPL$dispA(lc, o7, VAL, __loc0 = 12, VAL, g0, VAL)) && 
	((SUB$imode(lc, SP, VAL, __loc0, VAR, SP, VAL)) || true)) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("leaf_ret",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::ret_struct4(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	if (
false) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("ret_struct4",params);
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
		params.push_back("addr");
		theSemTable.addItem("addr");
		params.push_back("imm22");
		theSemTable.addItem("imm22");
		this->newLogue("struct_call","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("addr");
		theSemTable.addItem("addr");
		params.push_back("imm22");
		theSemTable.addItem("imm22");
		params.push_back("rs1");
		theSemTable.addItem("rs1");
		params.push_back("rs2");
		theSemTable.addItem("rs2");
		params.push_back("rd");
		theSemTable.addItem("rd");
		this->newLogue("call_rst_ui_reg","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("addr");
		theSemTable.addItem("addr");
		params.push_back("imm22");
		theSemTable.addItem("imm22");
		params.push_back("rs1");
		theSemTable.addItem("rs1");
		params.push_back("imm");
		theSemTable.addItem("imm");
		params.push_back("rd");
		theSemTable.addItem("rd");
		this->newLogue("call_rst_ui_imm","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("addr");
		theSemTable.addItem("addr");
		params.push_back("rs1");
		theSemTable.addItem("rs1");
		params.push_back("rs2");
		theSemTable.addItem("rs2");
		params.push_back("rd");
		theSemTable.addItem("rd");
		this->newLogue("call_restore_reg","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("addr");
		theSemTable.addItem("addr");
		params.push_back("rs1");
		theSemTable.addItem("rs1");
		params.push_back("imm");
		theSemTable.addItem("imm");
		params.push_back("rd");
		theSemTable.addItem("rd");
		this->newLogue("call_restore_imm","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("addr");
		theSemTable.addItem("addr");
		params.push_back("rd");
		theSemTable.addItem("rd");
		this->newLogue("move_call_move","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("addr");
		theSemTable.addItem("addr");
		params.push_back("rd");
		theSemTable.addItem("rd");
		this->newLogue("move_x_call_move","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("addr");
		theSemTable.addItem("addr");
		params.push_back("imm");
		theSemTable.addItem("imm");
		this->newLogue("call_add","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("rs1j");
		theSemTable.addItem("rs1j");
		params.push_back("rdj");
		theSemTable.addItem("rdj");
		params.push_back("rs1");
		theSemTable.addItem("rs1");
		params.push_back("rs2");
		theSemTable.addItem("rs2");
		params.push_back("rd");
		theSemTable.addItem("rd");
		this->newLogue("jmp_restore_reg","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("rs1j");
		theSemTable.addItem("rs1j");
		params.push_back("rdj");
		theSemTable.addItem("rdj");
		params.push_back("rs1");
		theSemTable.addItem("rs1");
		params.push_back("imm");
		theSemTable.addItem("imm");
		params.push_back("rd");
		theSemTable.addItem("rd");
		this->newLogue("jmp_restore_imm","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("locals");
		theSemTable.addItem("locals");
		this->newLogue("new_reg_win","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("locals");
		theSemTable.addItem("locals");
		this->newLogue("same_reg_win","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("hiVal");
		theSemTable.addItem("hiVal");
		params.push_back("loVal");
		theSemTable.addItem("loVal");
		params.push_back("reg");
		theSemTable.addItem("reg");
		this->newLogue("new_reg_win_large","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("hiVal");
		theSemTable.addItem("hiVal");
		params.push_back("loVal");
		theSemTable.addItem("loVal");
		params.push_back("reg");
		theSemTable.addItem("reg");
		this->newLogue("same_reg_win_large","CALLEE_PROLOGUE",params);
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
		params.push_back("rs1");
		theSemTable.addItem("rs1");
		params.push_back("rs2");
		theSemTable.addItem("rs2");
		this->newLogue("ret_reg_val","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		params.push_back("rs1");
		theSemTable.addItem("rs1");
		params.push_back("imm");
		theSemTable.addItem("imm");
		this->newLogue("ret_imm_val","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("leaf_ret","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("ret_struct4","CALLEE_EPILOGUE",params);
	}
}


