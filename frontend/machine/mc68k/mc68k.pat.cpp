#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 2 "machine/mc68k/mc68k.pat.m"
/*==============================================
 * FILE:      mc68k.pat.m
 * OVERVIEW:  Generated file; do not edit
 *==============================================*/

#include "global.h"
#include "decoder.h"
#include "mc68k.pat.h"
#include "ss.h"
#include "csr.h"

#define VAR true
#define VAL false
int InstructionPatterns::SP = 7;
int InstructionPatterns::FP = 6;
bool InstructionPatterns::Aline(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 20 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 20 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 10) { 
      unsigned _a = (MATCH_w_16_0 & 0xfff) /* bot12 at 0 */;
      nextPC = 2 + MATCH_p; 
      
      #line 22 "machine/mc68k/mc68k.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else { 
      nextPC = MATCH_p; 
      
      #line 25 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_r; 
  
  MATCH_finished_r: (void)0; /*placeholder for label*/
  
}

#line 29 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::addaw_d16(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 31 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 31 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if (0 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 13 || 
      14 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 16 || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 13 && 
      (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 4 || 5 <= (MATCH_w_16_0 & 0x7) 
            /* reg2 at 0 */ && (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 8) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 13 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 4 && 
      (0 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 7) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 13 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 4 && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 7 && 
      (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 7) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 13 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 4 && (MATCH_w_16_0 >> 3 & 0x7) 
            /* adrm at 0 */ == 7 && (MATCH_w_16_0 >> 9 & 0x7) 
            /* reg1 at 0 */ == 7 && (MATCH_w_16_0 >> 8 & 0x1) 
            /* sb at 0 */ == 0 && (0 <= (MATCH_w_16_0 >> 6 & 0x3) 
            /* sz at 0 */ && (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ < 3) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 13 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 4 && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 7 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7 && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
      goto MATCH_label_q0;  /*opt-block+*/
    else { 
      MATCH_w_16_16 = getWord(2 + MATCH_p); 
      { 
        int /* [~32768..32767] */ _a = 
          sign_extend((MATCH_w_16_16 & 0xffff) /* d16 at 16 */, 16);
        nextPC = 4 + MATCH_p; 
        
        #line 33 "machine/mc68k/mc68k.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    
  }goto MATCH_finished_q; 
  
  MATCH_label_q0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 36 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_q; 
    
  MATCH_finished_q: (void)0; /*placeholder for label*/
  
}

#line 40 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::addil$daIndirect(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 42 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 42 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  unsigned /* [0..65535] */ MATCH_w_16_32;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 3 || 
      4 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 8 || 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 3 && 
      (0 <= (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ < 2 || 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) || 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 3 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 2 && 
      ((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
      2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 0 && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1 || 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 3 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 2 && 
      ((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
      2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
      (1 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 16) || 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 3 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 2 && 
      ((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 1 || 
      5 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 8)) 
      goto MATCH_label_p0;  /*opt-block+*/
    else { 
      MATCH_w_16_16 = getWord(2 + MATCH_p); 
      MATCH_w_16_32 = getWord(4 + MATCH_p); 
      { 
        unsigned _a = 
          ((MATCH_w_16_16 & 0xffff) /* d16 at 16 */ << 16) + 
          (MATCH_w_16_32 & 0xffff) /* d16 at 32 */;
        unsigned _b = addressToPC(MATCH_p);
        nextPC = 6 + MATCH_p; 
        
        #line 44 "machine/mc68k/mc68k.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		if (!daIndirect(_b, b, b_isVAR)) return false;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    
  }goto MATCH_finished_p; 
  
  MATCH_label_p0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 48 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_p; 
    
  MATCH_finished_p: (void)0; /*placeholder for label*/
  
}

#line 52 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::bsr(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 54 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 54 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 8 & 0xf) /* cond at 0 */ == 1 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 6) { 
      unsigned _a = addressToPC(MATCH_p);
      nextPC = 2 + MATCH_p; 
      
      #line 56 "machine/mc68k/mc68k.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_o0;  /*opt-block+*/
    
  }goto MATCH_finished_o; 
  
  MATCH_label_o0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 59 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_o; 
    
  MATCH_finished_o: (void)0; /*placeholder for label*/
  
}

#line 63 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::call_(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 65 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 65 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if (0 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 4 || 
      5 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 16 || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2 || 3 <= (MATCH_w_16_0 & 0x7) 
            /* reg2 at 0 */ && (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 8) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && (MATCH_w_16_0 & 0x7) 
            /* reg2 at 0 */ == 2 && (0 <= (MATCH_w_16_0 >> 6 & 0x3) 
            /* sz at 0 */ && (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ < 2 || 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 2 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 2 && 
      (0 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 7) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 2 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 2 && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 7 && 
      (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 7) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 2 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 2 && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 7 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7 && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
      goto MATCH_label_n0;  /*opt-block+*/
    else { 
      MATCH_w_16_16 = getWord(2 + MATCH_p); 
      { 
        unsigned _a = 
          2 + sign_extend((MATCH_w_16_16 & 0xffff) /* d16 at 16 */, 16) + 
          addressToPC(MATCH_p);
        nextPC = 4 + MATCH_p; 
        
        #line 67 "machine/mc68k/mc68k.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    
  }goto MATCH_finished_n; 
  
  MATCH_label_n0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 70 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_n; 
    
  MATCH_finished_n: (void)0; /*placeholder for label*/
  
}

#line 73 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::daIndirect(ADDRESS& lc, int& a, bool a_isVAR) {


#line 75 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 75 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 2) { 
      unsigned _a = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 77 "machine/mc68k/mc68k.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      
      #line 79 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
       /*opt-block+*/
    
  }goto MATCH_finished_m; 
  
  MATCH_finished_m: (void)0; /*placeholder for label*/
  
}

#line 82 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::daPostInc(ADDRESS& lc, int& a, bool a_isVAR) {


#line 84 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 84 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 3) { 
      unsigned _a = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 86 "machine/mc68k/mc68k.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      
      #line 88 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
       /*opt-block+*/
    
  }goto MATCH_finished_l; 
  
  MATCH_finished_l: (void)0; /*placeholder for label*/
  
}

#line 91 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::daPreDec(ADDRESS& lc, int& a, bool a_isVAR) {


#line 93 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 93 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 4) { 
      unsigned _a = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 95 "machine/mc68k/mc68k.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      
      #line 97 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
       /*opt-block+*/
    
  }goto MATCH_finished_k; 
  
  MATCH_finished_k: (void)0; /*placeholder for label*/
  
}

#line 101 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::leaSpSp(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 103 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 103 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if (0 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5 || 
      6 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 8 || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 5 && 
      (0 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 4 || 
      5 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 16) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 5 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 7) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 5 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7 && 
      (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 7) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 5 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 7 && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 0 || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 5 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 7 && (MATCH_w_16_0 >> 8 & 0x1) 
            /* sb at 0 */ == 1 && (0 <= (MATCH_w_16_0 >> 6 & 0x3) 
            /* sz at 0 */ && (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ < 3)) 
      goto MATCH_label_j0;  /*opt-block+*/
    else { 
      MATCH_w_16_16 = getWord(2 + MATCH_p); 
      { 
        int /* [~32768..32767] */ _a = 
          sign_extend((MATCH_w_16_16 & 0xffff) /* d16 at 16 */, 16);
        nextPC = 4 + MATCH_p; 
        
        #line 105 "machine/mc68k/mc68k.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    
  }goto MATCH_finished_j; 
  
  MATCH_label_j0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 108 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_j; 
    
  MATCH_finished_j: (void)0; /*placeholder for label*/
  
}

#line 112 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::link(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 114 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 114 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if (0 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 2 || 
      3 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 8 || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 2 && 
      (0 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 4 || 
      5 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 16) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 2 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      ((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 0 || 
      2 <= (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ < 4) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 2 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 1 && 
      (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 7) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 2 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 1 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7 && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
      goto MATCH_label_i0;  /*opt-block+*/
    else { 
      MATCH_w_16_16 = getWord(2 + MATCH_p); 
      { 
        unsigned _a = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
        int /* [~32768..32767] */ _b = 
          sign_extend((MATCH_w_16_16 & 0xffff) /* d16 at 16 */, 16);
        nextPC = 4 + MATCH_p; 
        
        #line 116 "machine/mc68k/mc68k.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		if (!b_isVAR && (int)_b != b) return false; else b = _b;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    
  }goto MATCH_finished_i; 
  
  MATCH_label_i0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 120 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_i; 
    
  MATCH_finished_i: (void)0; /*placeholder for label*/
  
}

#line 124 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::movemrl$daPostInc(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 126 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 126 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if (0 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 4 || 
      5 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 16 || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 6 || 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 6 && 
      (0 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 2 || 
      4 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 8) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 6 && 
      (2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 4) && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 0 && 
      (0 <= (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ < 3) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 6 && 
      (2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 4) && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
      goto MATCH_label_h0;  /*opt-block+*/
    else { 
      MATCH_w_16_16 = getWord(2 + MATCH_p); 
      { 
        unsigned _a = addressToPC(MATCH_p);
        unsigned _b = (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
        nextPC = 4 + MATCH_p; 
        
        #line 128 "machine/mc68k/mc68k.pat.m"
        

        		if (!daPostInc(_a, a, a_isVAR)) return false;

        		if (!b_isVAR && (int)_b != b) return false; else b = _b;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    
  }goto MATCH_finished_h; 
  
  MATCH_label_h0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 132 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_h; 
    
  MATCH_finished_h: (void)0; /*placeholder for label*/
  
}

#line 136 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::moverml$daPreDec(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 138 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 138 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 2 || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 4) && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 4 && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 0 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
      MATCH_w_16_16 = getWord(2 + MATCH_p); 
      { 
        unsigned _a = (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
        unsigned _b = addressToPC(MATCH_p);
        nextPC = 4 + MATCH_p; 
        
        #line 140 "machine/mc68k/mc68k.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		if (!daPreDec(_b, b, b_isVAR)) return false;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    else 
      goto MATCH_label_g0;  /*opt-block+*/
    
  }goto MATCH_finished_g; 
  
  MATCH_label_g0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 144 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_g; 
    
  MATCH_finished_g: (void)0; /*placeholder for label*/
  
}

#line 148 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::peaPcDisp(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 150 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 150 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if (0 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 4 || 
      5 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 16 || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 4 || 
      5 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 8) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 4 && 
      (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2 || 3 <= (MATCH_w_16_0 & 0x7) 
            /* reg2 at 0 */ && (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 8) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 4 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 2 && ((MATCH_w_16_0 >> 6 & 0x3) 
            /* sz at 0 */ == 0 || 2 <= (MATCH_w_16_0 >> 6 & 0x3) 
            /* sz at 0 */ && (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ < 4) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 4 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 2 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 1 && 
      (0 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 7) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 4 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 2 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 1 && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 7 && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
      goto MATCH_label_f0;  /*opt-block+*/
    else { 
      MATCH_w_16_16 = getWord(2 + MATCH_p); 
      { 
        int /* [~32768..32767] */ _a = 
          sign_extend((MATCH_w_16_16 & 0xffff) /* d16 at 16 */, 16);
        nextPC = 4 + MATCH_p; 
        
        #line 152 "machine/mc68k/mc68k.pat.m"
        

        		if (!a_isVAR && (int)_a != a) return false; else a = _a;

        		lc = nextPC;

        		return true;

        
        
        
      }
      
    } /*opt-block*/
    
  }goto MATCH_finished_f; 
  
  MATCH_label_f0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 155 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_f; 
    
  MATCH_finished_f: (void)0; /*placeholder for label*/
  
}

#line 159 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::popreg(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 161 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 161 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 3 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 2 && 
      (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ == 0 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 7) { 
      unsigned _a = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      nextPC = 2 + MATCH_p; 
      
      #line 163 "machine/mc68k/mc68k.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_e0;  /*opt-block+*/
    
  }goto MATCH_finished_e; 
  
  MATCH_label_e0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 166 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_e; 
    
  MATCH_finished_e: (void)0; /*placeholder for label*/
  
}

#line 170 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::pushreg(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 172 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 172 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ == 4 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 2 && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7) { 
      unsigned _a = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      nextPC = 2 + MATCH_p; 
      
      #line 174 "machine/mc68k/mc68k.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      goto MATCH_label_d0;  /*opt-block+*/
    
  }goto MATCH_finished_d; 
  
  MATCH_label_d0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 177 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_d; 
    
  MATCH_finished_d: (void)0; /*placeholder for label*/
  
}

#line 181 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::rts(ADDRESS& lc) {
	ADDRESS nextPC;


#line 183 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 183 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if (0 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 6 || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 7 || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 6 && 
      (0 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 4 || 
      5 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 16) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 6 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 5 || 6 <= (MATCH_w_16_0 & 0x7) 
            /* reg2 at 0 */ && (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 8) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 6 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 5 && ((MATCH_w_16_0 >> 6 & 0x3) 
            /* sz at 0 */ == 0 || 2 <= (MATCH_w_16_0 >> 6 & 0x3) 
            /* sz at 0 */ && (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ < 4) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 6 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 5 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 1 && 
      (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 7) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 6 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ == 5 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 1 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7 && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
      goto MATCH_label_c0;  /*opt-block+*/
    else { 
      nextPC = 2 + MATCH_p; 
      
      #line 185 "machine/mc68k/mc68k.pat.m"
      

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_c; 
  
  MATCH_label_c0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 187 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_c; 
    
  MATCH_finished_c: (void)0; /*placeholder for label*/
  
}

#line 191 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::trap(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 193 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 193 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if (0 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 4 || 
      5 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 16 || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      ((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 0 || 
      2 <= (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ < 4) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 1 && 
      (MATCH_w_16_0 >> 4 & 0x3) /* adrb at 0 */ == 0 && 
      (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 7) || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 1 && 
      (MATCH_w_16_0 >> 4 & 0x3) /* adrb at 0 */ == 0 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7 && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1 || 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 1 && 
      (1 <= (MATCH_w_16_0 >> 4 & 0x3) /* adrb at 0 */ && 
      (MATCH_w_16_0 >> 4 & 0x3) /* adrb at 0 */ < 4)) 
      goto MATCH_label_b0;  /*opt-block+*/
    else { 
      unsigned _a = (MATCH_w_16_0 & 0xf) /* vect at 0 */;
      nextPC = 2 + MATCH_p; 
      
      #line 195 "machine/mc68k/mc68k.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_b; 
  
  MATCH_label_b0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 198 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_b; 
    
  MATCH_finished_b: (void)0; /*placeholder for label*/
  
}

#line 202 "machine/mc68k/mc68k.pat.m"
}
bool InstructionPatterns::unlk(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;


#line 204 "machine/mc68k/mc68k.pat.m"
{ 
  dword MATCH_p = 
    
    #line 204 "machine/mc68k/mc68k.pat.m"
    lc
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if (0 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 3 || 
      4 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 8 || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 3 && 
      (0 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 4 || 
      5 <= (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ < 16) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 3 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      ((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 0 || 
      2 <= (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ < 4) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 3 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 1 && 
      (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 7) || 
      (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 3 && 
      (MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 4 && 
      (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 1 && 
      (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7 && 
      (MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
      goto MATCH_label_a0;  /*opt-block+*/
    else { 
      unsigned _a = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      nextPC = 2 + MATCH_p; 
      
      #line 206 "machine/mc68k/mc68k.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 209 "machine/mc68k/mc68k.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 213 "machine/mc68k/mc68k.pat.m"
}
Logue* InstructionPatterns::std_call(CSR& csr, ADDRESS& lc, int& addr)
{
	ADDRESS __save = lc;
	if (
	call_(lc, addr, VAR)) {
		vector<int> params(1); params[0] = addr; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("std_call",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::near_call(CSR& csr, ADDRESS& lc, int& addr)
{
	ADDRESS __save = lc;
	if (
	bsr(lc, addr, VAR)) {
		vector<int> params(1); params[0] = addr; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("near_call",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::pea_add_rts(CSR& csr, ADDRESS& lc, int& d32)
{
	ADDRESS __save = lc;
	int __loc0;
	if (
	peaPcDisp(lc, __loc0 = 4, VAL) && 
	addil$daIndirect(lc, d32, VAR, SP, VAL) && 
	rts(lc)) {
		vector<int> params(1); params[0] = d32; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("pea_add_rts",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::pea_pea_add_rts(CSR& csr, ADDRESS& lc, int& d32)
{
	ADDRESS __save = lc;
	int __loc0;
	if (
	peaPcDisp(lc, __loc0 = 14, VAL) && 
	peaPcDisp(lc, __loc0 = 4, VAL) && 
	addil$daIndirect(lc, d32, VAR, SP, VAL) && 
	rts(lc)) {
		vector<int> params(1); params[0] = d32; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("pea_pea_add_rts",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::trap_syscall(CSR& csr, ADDRESS& lc, int& d16)
{
	ADDRESS __save = lc;
	int __loc0;
	if (
	trap(lc, __loc0 = 15, VAL) && 
	Aline(lc, d16, VAR)) {
		vector<int> params(1); params[0] = d16; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("trap_syscall",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::std_link(CSR& csr, ADDRESS& lc, int& locals)
{
	ADDRESS __save = lc;
	if (
	link(lc, FP, VAL, locals, VAR)) {
		vector<int> params(1); params[0] = locals; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("std_link",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::link_save(CSR& csr, ADDRESS& lc, int& locals, int& d16)
{
	ADDRESS __save = lc;
	if (
	link(lc, FP, VAL, locals, VAR) && 
	moverml$daPreDec(lc, d16, VAR, SP, VAL)) {
		vector<int> params(2); params[0] = locals; params[1] = d16; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("link_save",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::link_save1(CSR& csr, ADDRESS& lc, int& locals, int& reg)
{
	ADDRESS __save = lc;
	if (
	link(lc, FP, VAL, locals, VAR) && 
	pushreg(lc, reg, VAR)) {
		vector<int> params(2); params[0] = locals; params[1] = reg; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("link_save1",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::push_lea(CSR& csr, ADDRESS& lc, int& locals, int& reg)
{
	ADDRESS __save = lc;
	if (
	pushreg(lc, reg, VAR) && 
	leaSpSp(lc, locals, VAR)) {
		vector<int> params(2); params[0] = locals; params[1] = reg; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("push_lea",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::bare_ret(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	if (
	rts(lc)) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("bare_ret",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::std_ret(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	if (
	unlk(lc, FP, VAL) && 
	rts(lc)) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("std_ret",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::rest_ret(CSR& csr, ADDRESS& lc, int& d16)
{
	ADDRESS __save = lc;
	if (
	movemrl$daPostInc(lc, SP, VAL, d16, VAR) && 
	unlk(lc, FP, VAL) && 
	rts(lc)) {
		vector<int> params(1); params[0] = d16; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("rest_ret",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::rest1_ret(CSR& csr, ADDRESS& lc, int& reg)
{
	ADDRESS __save = lc;
	if (
	popreg(lc, reg, VAR) && 
	unlk(lc, FP, VAL) && 
	rts(lc)) {
		vector<int> params(1); params[0] = reg; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("rest1_ret",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::pop_ret(CSR& csr, ADDRESS& lc, int& reg)
{
	ADDRESS __save = lc;
	if (
	popreg(lc, reg, VAR) && 
	rts(lc)) {
		vector<int> params(1); params[0] = reg; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("pop_ret",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::clear_stack(CSR& csr, ADDRESS& lc, int& n)
{
	ADDRESS __save = lc;
	if (
	(leaSpSp(lc, n, VAR) || 
	addaw_d16(lc, n, VAR))) {
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
		params.push_back("addr");
		theSemTable.addItem("addr");
		this->newLogue("near_call","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("d32");
		theSemTable.addItem("d32");
		this->newLogue("pea_add_rts","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("d32");
		theSemTable.addItem("d32");
		this->newLogue("pea_pea_add_rts","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("d16");
		theSemTable.addItem("d16");
		this->newLogue("trap_syscall","CALLER_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("locals");
		theSemTable.addItem("locals");
		this->newLogue("std_link","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("locals");
		theSemTable.addItem("locals");
		params.push_back("d16");
		theSemTable.addItem("d16");
		this->newLogue("link_save","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("locals");
		theSemTable.addItem("locals");
		params.push_back("reg");
		theSemTable.addItem("reg");
		this->newLogue("link_save1","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("locals");
		theSemTable.addItem("locals");
		params.push_back("reg");
		theSemTable.addItem("reg");
		this->newLogue("push_lea","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("bare_ret","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("std_ret","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		params.push_back("d16");
		theSemTable.addItem("d16");
		this->newLogue("rest_ret","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		params.push_back("reg");
		theSemTable.addItem("reg");
		this->newLogue("rest1_ret","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		params.push_back("reg");
		theSemTable.addItem("reg");
		this->newLogue("pop_ret","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		params.push_back("n");
		theSemTable.addItem("n");
		this->newLogue("clear_stack","CALLER_EPILOGUE",params);
	}
}


