#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 2 "machine/hppa/hppa.pat.m"
/*==============================================
 * FILE:      hppa.pat.m
 * OVERVIEW:  Generated file; do not edit
 *
 * (C) 1998-2000 The University of Queensland, BT group
 *==============================================*/

#include "global.h"
#include "decoder.h"
#include "hppa.pat.h"
#include "ss.h"
#include "csr.h"

#define VAR true
#define VAL false
int InstructionPatterns::R0 = 0;
int InstructionPatterns::RP = 2;
int InstructionPatterns::SP = 30;
int InstructionPatterns::R31 = 31;
bool InstructionPatterns::BL$c_br_nnull(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 22 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 22 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 58 || 
      59 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 58 && 
      (1 <= (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ < 8)) 
      goto MATCH_label_y0;  /*opt-block+*/
    else { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = 
        8 + ((MATCH_w_32_0 >> 3 & 0x3ff) /* w10_19 at 0 */ << 2) + 
        ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 12) + 
        ((MATCH_w_32_0 >> 16 & 0x1f) /* w5_11 at 0 */ << 13) + 
        (sign_extend((MATCH_w_32_0 & 0x1) /* w_31 at 0 */, 1) << 18);
      unsigned _c = (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 24 "machine/hppa/hppa.pat.m"
      

      		if (!c_br_nnull(_a)) return false;

      		if (!a_isVAR && (int)_b != a) return false; else a = _b;

      		if (!b_isVAR && (int)_c != b) return false; else b = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_y; 
  
  MATCH_label_y0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 29 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_y; 
    
  MATCH_finished_y: (void)0; /*placeholder for label*/
  
}

#line 33 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::BL$c_br_null(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 35 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 35 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 58 || 
      59 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 58 && 
      (1 <= (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ < 8)) 
      goto MATCH_label_x0;  /*opt-block+*/
    else { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = 
        8 + ((MATCH_w_32_0 >> 3 & 0x3ff) /* w10_19 at 0 */ << 2) + 
        ((MATCH_w_32_0 >> 2 & 0x1) /* w_29 at 0 */ << 12) + 
        ((MATCH_w_32_0 >> 16 & 0x1f) /* w5_11 at 0 */ << 13) + 
        (sign_extend((MATCH_w_32_0 & 0x1) /* w_31 at 0 */, 1) << 18);
      unsigned _c = (MATCH_w_32_0 >> 21 & 0x1f) /* t_06 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 37 "machine/hppa/hppa.pat.m"
      

      		if (!c_br_null(_a)) return false;

      		if (!a_isVAR && (int)_b != a) return false; else a = _b;

      		if (!b_isVAR && (int)_c != b) return false; else b = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_x; 
  
  MATCH_label_x0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 42 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_x; 
    
  MATCH_finished_x: (void)0; /*placeholder for label*/
  
}

#line 46 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::BV$c_br_nnull(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 48 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 48 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ < 6 || 
      (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ == 7 || 
      (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ == 6 && 
      (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 58 || 
      59 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64) || 
      (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ == 6 && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 58 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* ve_19 at 0 */ == 1) 
      goto MATCH_label_w0;  /*opt-block+*/
    else { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;
      unsigned _c = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 50 "machine/hppa/hppa.pat.m"
      

      		if (!c_br_nnull(_a)) return false;

      		if (!a_isVAR && (int)_b != a) return false; else a = _b;

      		if (!b_isVAR && (int)_c != b) return false; else b = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_w; 
  
  MATCH_label_w0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 55 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_w; 
    
  MATCH_finished_w: (void)0; /*placeholder for label*/
  
}

#line 59 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::BV$c_br_null(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;


#line 61 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 61 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ && 
      (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ < 6 || 
      (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ == 7 || 
      (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ == 6 && 
      (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 58 || 
      59 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64) || 
      (MATCH_w_32_0 >> 13 & 0x7) /* ext3_16 at 0 */ == 6 && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 58 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* ve_19 at 0 */ == 1) 
      goto MATCH_label_v0;  /*opt-block+*/
    else { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_32_0 >> 16 & 0x1f) /* x_11 at 0 */;
      unsigned _c = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 63 "machine/hppa/hppa.pat.m"
      

      		if (!c_br_null(_a)) return false;

      		if (!a_isVAR && (int)_b != a) return false; else a = _b;

      		if (!b_isVAR && (int)_c != b) return false; else b = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_v; 
  
  MATCH_label_v0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 68 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_v; 
    
  MATCH_finished_v: (void)0; /*placeholder for label*/
  
}

#line 72 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::FSTDS$s_addr_im_r$c_s_addr_ma(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;


#line 74 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 74 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 11 || 
      12 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 9 & 0x1) /* addr_22 at 0 */ == 1 && 
      (1 <= (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ && 
      (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ < 4)) 
      goto MATCH_label_u0;  /*opt-block+*/
    else 
      goto MATCH_label_u1;  /*opt-block+*/
    
  }goto MATCH_finished_u; 
  
  MATCH_label_u0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 83 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_u; 
    
  MATCH_label_u1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_32_0 & 0x1f) /* r_27 at 0 */;
      unsigned _c = addressToPC(MATCH_p);
      unsigned _d = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned _e = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 76 "machine/hppa/hppa.pat.m"
      

      		if (!c_s_addr_ma(_a)) return false;

      		if (!a_isVAR && (int)_b != a) return false; else a = _b;

      		if (!s_addr_im_r$c_s_addr_ma(_c, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_d != c) return false; else c = _d;

      		if (!d_isVAR && (int)_e != d) return false; else d = _e;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_u; 
    
  MATCH_finished_u: (void)0; /*placeholder for label*/
  
}

#line 87 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::LDO(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;


#line 89 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 89 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 13) { 
      unsigned _a = 
        (sign_extend((MATCH_w_32_0 & 0x1) /* i_31 at 0 */, 1) << 13) + 
        (MATCH_w_32_0 >> 1 & 0x1fff) /* im13_18 at 0 */;
      unsigned _b = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned _c = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 91 "machine/hppa/hppa.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		if (!c_isVAR && (int)_c != c) return false; else c = _c;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else { 
      nextPC = MATCH_p; 
      
      #line 96 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_t; 
  
  MATCH_finished_t: (void)0; /*placeholder for label*/
  
}

#line 100 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::LDW$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;


#line 102 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 102 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 18) { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned _d = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned _e = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 104 "machine/hppa/hppa.pat.m"
      

      		if (!c_l_addr_none(_a)) return false;

      		if (!l_addr_16_old$c_l_addr_none(_b, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_c != b) return false; else b = _c;

      		if (!c_isVAR && (int)_d != c) return false; else c = _d;

      		if (!d_isVAR && (int)_e != d) return false; else d = _e;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else { 
      nextPC = MATCH_p; 
      
      #line 111 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_s; 
  
  MATCH_finished_s: (void)0; /*placeholder for label*/
  
}

#line 115 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::LDWM$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;


#line 117 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 117 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 19) { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned _d = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned _e = (MATCH_w_32_0 >> 16 & 0x1f) /* t_11 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 119 "machine/hppa/hppa.pat.m"
      

      		if (!c_l_addr_none(_a)) return false;

      		if (!l_addr_16_old$c_l_addr_none(_b, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_c != b) return false; else b = _c;

      		if (!c_isVAR && (int)_d != c) return false; else c = _d;

      		if (!d_isVAR && (int)_e != d) return false; else d = _e;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else { 
      nextPC = MATCH_p; 
      
      #line 126 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_r; 
  
  MATCH_finished_r: (void)0; /*placeholder for label*/
  
}

#line 130 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::LDWS$s_addr_im_r$c_s_addr_mb(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;


#line 132 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 132 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 2 || 
      3 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 16 || 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ == 2 && 
      (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 3 || 
      4 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64) || 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ == 2 && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0) 
      goto MATCH_label_q0;  /*opt-block+*/
    else 
      goto MATCH_label_q1;  /*opt-block+*/
    
  }goto MATCH_finished_q; 
  
  MATCH_label_q0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 141 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_q; 
    
  MATCH_label_q1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned _d = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned _e = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 134 "machine/hppa/hppa.pat.m"
      

      		if (!c_s_addr_mb(_a)) return false;

      		if (!s_addr_im_r$c_s_addr_mb(_b, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_c != b) return false; else b = _c;

      		if (!c_isVAR && (int)_d != c) return false; else c = _d;

      		if (!d_isVAR && (int)_e != d) return false; else d = _e;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_q; 
    
  MATCH_finished_q: (void)0; /*placeholder for label*/
  
}

#line 145 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::LDWS$s_addr_im_r$c_s_addr_notm(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;


#line 147 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 147 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 2 || 
      3 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 16 || 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ == 2 && 
      (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 3 || 
      4 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64) || 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ == 2 && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0) 
      goto MATCH_label_p0;  /*opt-block+*/
    else 
      goto MATCH_label_p1;  /*opt-block+*/
    
  }goto MATCH_finished_p; 
  
  MATCH_label_p0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 156 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_p; 
    
  MATCH_label_p1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = addressToPC(MATCH_p);
      unsigned _c = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned _d = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      unsigned _e = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 149 "machine/hppa/hppa.pat.m"
      

      		if (!c_s_addr_notm(_a)) return false;

      		if (!s_addr_im_r$c_s_addr_notm(_b, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_c != b) return false; else b = _c;

      		if (!c_isVAR && (int)_d != c) return false; else c = _d;

      		if (!d_isVAR && (int)_e != d) return false; else d = _e;

      		lc = nextPC;

      		return true;

      
      
      
    } 
    goto MATCH_finished_p; 
    
  MATCH_finished_p: (void)0; /*placeholder for label*/
  
}

#line 160 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::OR$c_arith_w$c_c_nonneg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;


#line 162 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 162 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 9 || 
      10 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 64 || 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ == 9 && 
      (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 2 || 
      3 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64)) 
      goto MATCH_label_o0;  /*opt-block+*/
    else { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned _c = (MATCH_w_32_0 >> 21 & 0x1f) /* r_06 at 0 */;
      unsigned _d = (MATCH_w_32_0 & 0x1f) /* t_27 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 164 "machine/hppa/hppa.pat.m"
      

      		if (!c_arith_w$c_c_nonneg(_a, a, a_isVAR)) return false;

      		if (!b_isVAR && (int)_b != b) return false; else b = _b;

      		if (!c_isVAR && (int)_c != c) return false; else c = _c;

      		if (!d_isVAR && (int)_d != d) return false; else d = _d;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_o; 
  
  MATCH_label_o0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      
      #line 170 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } 
    goto MATCH_finished_o; 
    
  MATCH_finished_o: (void)0; /*placeholder for label*/
  
}

#line 174 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::STW$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;


#line 176 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 176 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 26) { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned _c = addressToPC(MATCH_p);
      unsigned _d = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned _e = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 178 "machine/hppa/hppa.pat.m"
      

      		if (!c_l_addr_none(_a)) return false;

      		if (!a_isVAR && (int)_b != a) return false; else a = _b;

      		if (!l_addr_16_old$c_l_addr_none(_c, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_d != c) return false; else c = _d;

      		if (!d_isVAR && (int)_e != d) return false; else d = _e;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else { 
      nextPC = MATCH_p; 
      
      #line 185 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_n; 
  
  MATCH_finished_n: (void)0; /*placeholder for label*/
  
}

#line 189 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::STWM$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;


#line 191 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 191 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 27) { 
      unsigned _a = addressToPC(MATCH_p);
      unsigned _b = (MATCH_w_32_0 >> 16 & 0x1f) /* r_11 at 0 */;
      unsigned _c = addressToPC(MATCH_p);
      unsigned _d = (MATCH_w_32_0 >> 14 & 0x3) /* s2_16 at 0 */;
      unsigned _e = (MATCH_w_32_0 >> 21 & 0x1f) /* b_06 at 0 */;
      nextPC = 4 + MATCH_p; 
      
      #line 193 "machine/hppa/hppa.pat.m"
      

      		if (!c_l_addr_none(_a)) return false;

      		if (!a_isVAR && (int)_b != a) return false; else a = _b;

      		if (!l_addr_16_old$c_l_addr_none(_c, b, b_isVAR)) return false;

      		if (!c_isVAR && (int)_d != c) return false; else c = _d;

      		if (!d_isVAR && (int)_e != d) return false; else d = _e;

      		lc = nextPC;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else { 
      nextPC = MATCH_p; 
      
      #line 200 "machine/hppa/hppa.pat.m"
      
      		return false;

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_m; 
  
  MATCH_finished_m: (void)0; /*placeholder for label*/
  
}

#line 203 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::c_arith_w$c_c_nonneg(ADDRESS& lc, int& a, bool a_isVAR) {


#line 205 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 205 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 2 && 
      ((MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ == 0 || 
      4 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 6 || 
      7 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 18 || 
      19 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 32 || 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ == 34 || 
      38 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 44 || 
      46 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 49 || 
      51 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 53 || 
      56 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 61) && 
      (MATCH_w_32_0 >> 5 & 0x1) /* d_26 at 0 */ == 0 && 
      (0 <= (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ && 
      (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ < 2) || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 36 && 
      (0 <= (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ && 
      (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ < 2) || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 37 || 
      44 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 46) && 
      (0 <= (MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ && 
      (MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ < 2) && 
      (0 <= (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ && 
      (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ < 2) || 
      40 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 44) 
      goto MATCH_label_l1;  /*opt-block+*/
    else 
      goto MATCH_label_l0;  /*opt-block+*/
    
  }goto MATCH_finished_l; 
  
  MATCH_label_l0: (void)0; /*placeholder for label*/ 
    
    #line 210 "machine/hppa/hppa.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_l; 
    
  MATCH_label_l1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = (MATCH_w_32_0 >> 13 & 0x7) /* c3_16 at 0 */;
      unsigned _b = addressToPC(MATCH_p);
      
      #line 207 "machine/hppa/hppa.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		if (!c_c_nonneg(_b)) return false;

      		return true;

      
      
      
    } 
    goto MATCH_finished_l; 
    
  MATCH_finished_l: (void)0; /*placeholder for label*/
  
}

#line 213 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::c_br_nnull(ADDRESS& lc) {


#line 215 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 215 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((32 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 36 || 
      39 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 44 || 
      47 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 52 || 
      56 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 60) && 
      (MATCH_w_32_0 >> 1 & 0x1) /* n_30 at 0 */ == 0) 
      
      #line 216 "machine/hppa/hppa.pat.m"
      

      		return true;

      
       /*opt-block+*/
    else 
      goto MATCH_label_k0;  /*opt-block+*/
    
  }goto MATCH_finished_k; 
  
  MATCH_label_k0: (void)0; /*placeholder for label*/ 
    
    #line 218 "machine/hppa/hppa.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_k; 
    
  MATCH_finished_k: (void)0; /*placeholder for label*/
  
}

#line 221 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::c_br_null(ADDRESS& lc) {


#line 223 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 223 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((32 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 36 || 
      39 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 44 || 
      47 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 52 || 
      56 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 60) && 
      (MATCH_w_32_0 >> 1 & 0x1) /* n_30 at 0 */ == 1) 
      
      #line 224 "machine/hppa/hppa.pat.m"
      

      		return true;

      
       /*opt-block+*/
    else 
      goto MATCH_label_j0;  /*opt-block+*/
    
  }goto MATCH_finished_j; 
  
  MATCH_label_j0: (void)0; /*placeholder for label*/ 
    
    #line 226 "machine/hppa/hppa.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_j; 
    
  MATCH_finished_j: (void)0; /*placeholder for label*/
  
}

#line 229 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::c_c_nonneg(ADDRESS& lc) {


#line 231 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 231 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 2 && 
      ((MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ == 0 || 
      4 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 6 || 
      7 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 18 || 
      19 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 32 || 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ == 34 || 
      38 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 44 || 
      46 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 49 || 
      51 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 53 || 
      56 <= (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0x3f) /* ext6_20 at 0 */ < 61) && 
      (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ == 0 || 
      (32 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 34 || 
      39 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 42) || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 36 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ == 0 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 37 || 
      44 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 46) && 
      (MATCH_w_32_0 >> 12 & 0x1) /* f_19 at 0 */ == 0 && 
      (0 <= (MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ && 
      (MATCH_w_32_0 >> 11 & 0x1) /* ext_20 at 0 */ < 2) || 
      (50 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 52 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 59) && 
      (MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 52 && 
      (MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 0 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 0 && 
      (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ == 0 && 
      (0 <= (MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ && 
      (MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ < 2) || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 52 && 
      (MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 0 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ == 1 && 
      (0 <= (MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ && 
      (MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ < 2) && 
      (0 <= (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ && 
      (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ < 2) || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 53 && 
      (MATCH_w_32_0 >> 15 & 0x1) /* c1_16 at 0 */ == 0 && 
      (0 <= (MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ && 
      (MATCH_w_32_0 >> 12 & 0x1) /* cl_19 at 0 */ < 2) && 
      (0 <= (MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ && 
      (MATCH_w_32_0 >> 11 & 0x1) /* p_20 at 0 */ < 2) && 
      (0 <= (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ && 
      (MATCH_w_32_0 >> 10 & 0x1) /* se_21 at 0 */ < 2)) 
      goto MATCH_label_i1;  /*opt-block+*/
    else 
      goto MATCH_label_i0;  /*opt-block+*/
    
  }goto MATCH_finished_i; 
  
  MATCH_label_i0: (void)0; /*placeholder for label*/ 
    
    #line 234 "machine/hppa/hppa.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_i; 
    
  MATCH_label_i1: (void)0; /*placeholder for label*/ 
    
    #line 232 "machine/hppa/hppa.pat.m"
    

    		return true;

    
     
    goto MATCH_finished_i; 
    
  MATCH_finished_i: (void)0; /*placeholder for label*/
  
}

#line 237 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::c_l_addr_none(ADDRESS& lc) {


#line 239 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 239 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (16 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 20 || 
      24 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 28) 
      
      #line 240 "machine/hppa/hppa.pat.m"
      

      		return true;

      
       /*opt-block+*/
    else 
      
      #line 242 "machine/hppa/hppa.pat.m"
      
      		return false;

      
       /*opt-block+*/
    
  }goto MATCH_finished_h; 
  
  MATCH_finished_h: (void)0; /*placeholder for label*/
  
}

#line 245 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::c_s_addr_ma(ADDRESS& lc) {


#line 247 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 247 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 3 || 
      4 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 10 || 
      12 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
      (12 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 14) || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
      (1 <= (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ && 
      (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ < 4) || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1) 
      goto MATCH_label_g0;  /*opt-block+*/
    else 
      goto MATCH_label_g1;  /*opt-block+*/
    
  }goto MATCH_finished_g; 
  
  MATCH_label_g0: (void)0; /*placeholder for label*/ 
    
    #line 250 "machine/hppa/hppa.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_g; 
    
  MATCH_label_g1: (void)0; /*placeholder for label*/ 
    
    #line 248 "machine/hppa/hppa.pat.m"
    

    		return true;

    
     
    goto MATCH_finished_g; 
    
  MATCH_finished_g: (void)0; /*placeholder for label*/
  
}

#line 253 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::c_s_addr_mb(ADDRESS& lc) {


#line 255 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 255 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 3 || 
      4 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 10 || 
      12 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
      (12 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 14) || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 0 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 13 & 0x1) /* a_18 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 && 
      (1 <= (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ && 
      (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ < 4)) 
      goto MATCH_label_f0;  /*opt-block+*/
    else 
      goto MATCH_label_f1;  /*opt-block+*/
    
  }goto MATCH_finished_f; 
  
  MATCH_label_f0: (void)0; /*placeholder for label*/ 
    
    #line 258 "machine/hppa/hppa.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_f; 
    
  MATCH_label_f1: (void)0; /*placeholder for label*/ 
    
    #line 256 "machine/hppa/hppa.pat.m"
    

    		return true;

    
     
    goto MATCH_finished_f; 
    
  MATCH_finished_f: (void)0; /*placeholder for label*/
  
}

#line 261 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::c_s_addr_notm(ADDRESS& lc) {


#line 263 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 263 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 3 || 
      4 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 10 || 
      12 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 && 
      (12 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 14) || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 0 && 
      (1 <= (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ && 
      (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ < 4) || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (MATCH_w_32_0 >> 5 & 0x1) /* m_26 at 0 */ == 1) 
      goto MATCH_label_e0;  /*opt-block+*/
    else 
      goto MATCH_label_e1;  /*opt-block+*/
    
  }goto MATCH_finished_e; 
  
  MATCH_label_e0: (void)0; /*placeholder for label*/ 
    
    #line 266 "machine/hppa/hppa.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_e; 
    
  MATCH_label_e1: (void)0; /*placeholder for label*/ 
    
    #line 264 "machine/hppa/hppa.pat.m"
    

    		return true;

    
     
    goto MATCH_finished_e; 
    
  MATCH_finished_e: (void)0; /*placeholder for label*/
  
}

#line 269 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR) {


#line 271 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 271 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (16 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 20 || 
      24 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 28) { 
      unsigned _a = 
        (sign_extend((MATCH_w_32_0 & 0x1) /* i_31 at 0 */, 1) << 13) + 
        (MATCH_w_32_0 >> 1 & 0x1fff) /* im13_18 at 0 */;
      
      #line 273 "machine/hppa/hppa.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      
      #line 275 "machine/hppa/hppa.pat.m"
      
      		return false;

      
       /*opt-block+*/
    
  }goto MATCH_finished_d; 
  
  MATCH_finished_d: (void)0; /*placeholder for label*/
  
}

#line 278 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::s_addr_im_r$c_s_addr_ma(ADDRESS& lc, int& a, bool a_isVAR) {


#line 280 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 280 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 3 || 
      4 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 10 || 
      12 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (8 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 16) || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (1 <= (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ && 
      (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ < 4)) 
      goto MATCH_label_c0;  /*opt-block+*/
    else 
      goto MATCH_label_c1;  /*opt-block+*/
    
  }goto MATCH_finished_c; 
  
  MATCH_label_c0: (void)0; /*placeholder for label*/ 
    
    #line 284 "machine/hppa/hppa.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_c; 
    
  MATCH_label_c1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 
        (sign_extend((MATCH_w_32_0 >> 16 & 0x1) /* im1_15 at 0 */, 1) << 4) + 
        (MATCH_w_32_0 >> 17 & 0xf) /* im4_11 at 0 */;
      
      #line 282 "machine/hppa/hppa.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } 
    goto MATCH_finished_c; 
    
  MATCH_finished_c: (void)0; /*placeholder for label*/
  
}

#line 287 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::s_addr_im_r$c_s_addr_mb(ADDRESS& lc, int& a, bool a_isVAR) {


#line 289 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 289 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 3 || 
      4 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 10 || 
      12 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (8 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 16) || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (1 <= (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ && 
      (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ < 4)) 
      goto MATCH_label_b0;  /*opt-block+*/
    else 
      goto MATCH_label_b1;  /*opt-block+*/
    
  }goto MATCH_finished_b; 
  
  MATCH_label_b0: (void)0; /*placeholder for label*/ 
    
    #line 293 "machine/hppa/hppa.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_b; 
    
  MATCH_label_b1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 
        (sign_extend((MATCH_w_32_0 >> 16 & 0x1) /* im1_15 at 0 */, 1) << 4) + 
        (MATCH_w_32_0 >> 17 & 0xf) /* im4_11 at 0 */;
      
      #line 291 "machine/hppa/hppa.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } 
    goto MATCH_finished_b; 
    
  MATCH_finished_b: (void)0; /*placeholder for label*/
  
}

#line 296 "machine/hppa/hppa.pat.m"
}
bool InstructionPatterns::s_addr_im_r$c_s_addr_notm(ADDRESS& lc, int& a, bool a_isVAR) {


#line 298 "machine/hppa/hppa.pat.m"
{ 
  dword MATCH_p = 
    
    #line 298 "machine/hppa/hppa.pat.m"
    lc
    ;
  unsigned MATCH_w_32_0;
  { 
    MATCH_w_32_0 = getDword(MATCH_p); 
    if (0 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 3 || 
      4 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 10 || 
      12 <= (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ && 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ < 64 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 3 && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (8 <= (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ && 
      (MATCH_w_32_0 >> 6 & 0xf) /* ext4_22 at 0 */ < 16) || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 0 || 
      ((MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 9 || 
      (MATCH_w_32_0 >> 26 & 0x3f) /* op at 0 */ == 11) && 
      (MATCH_w_32_0 >> 12 & 0x1) /* addr_19 at 0 */ == 1 && 
      (1 <= (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ && 
      (MATCH_w_32_0 >> 7 & 0x3) /* uid2_23 at 0 */ < 4)) 
      goto MATCH_label_a0;  /*opt-block+*/
    else 
      goto MATCH_label_a1;  /*opt-block+*/
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    
    #line 302 "machine/hppa/hppa.pat.m"
    
    		return false;

    
     
    goto MATCH_finished_a; 
    
  MATCH_label_a1: (void)0; /*placeholder for label*/ 
    { 
      unsigned _a = 
        (sign_extend((MATCH_w_32_0 >> 16 & 0x1) /* im1_15 at 0 */, 1) << 4) + 
        (MATCH_w_32_0 >> 17 & 0xf) /* im4_11 at 0 */;
      
      #line 300 "machine/hppa/hppa.pat.m"
      

      		if (!a_isVAR && (int)_a != a) return false; else a = _a;

      		return true;

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 306 "machine/hppa/hppa.pat.m"
}
Logue* InstructionPatterns::std_call(CSR& csr, ADDRESS& lc, int& addr)
{
	ADDRESS __save = lc;
	if (
	BL$c_br_nnull(lc, addr, VAR, RP, VAL)) {
		vector<int> params(1); params[0] = addr; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("std_call",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::gcc_frame(CSR& csr, ADDRESS& lc, int& locals)
{
	ADDRESS __save = lc;
	int __loc0;
	int __loc1;
	int __loc2;
	int __loc3;
	if (
	STW$l_addr_16_old$c_l_addr_none(lc, RP, VAL, __loc0 = -20, VAL, __loc1 = 0, VAL, SP, VAL) && 
	OR$c_arith_w$c_c_nonneg(lc, __loc0 = 0, VAL, __loc1 = 3, VAL, __loc2 = 0, VAL, __loc3 = 1, VAL) && 
	OR$c_arith_w$c_c_nonneg(lc, __loc0 = 0, VAL, SP, VAL, __loc1 = 0, VAL, __loc2 = 3, VAL) && 
	STWM$l_addr_16_old$c_l_addr_none(lc, __loc0 = 1, VAL, locals, VAR, __loc1 = 0, VAL, SP, VAL) && 
	((STW$l_addr_16_old$c_l_addr_none(lc, __loc0 = 4, VAL, __loc1 = 8, VAL, __loc2 = 0, VAL, __loc3 = 3, VAL)) || true)) {
		vector<int> params(1); params[0] = locals; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("gcc_frame",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::gcc_frameless(CSR& csr, ADDRESS& lc, int& locals)
{
	ADDRESS __save = lc;
	int __loc0;
	int __loc1;
	int __loc2;
	if (
	STW$l_addr_16_old$c_l_addr_none(lc, RP, VAL, __loc0 = -20, VAL, __loc1 = 0, VAL, SP, VAL) && 
	(LDO(lc, locals, VAR, SP, VAL, SP, VAL) || 
	STWM$l_addr_16_old$c_l_addr_none(lc, __loc0, VAR, locals, VAR, __loc1 = 0, VAL, SP, VAL)) && 
	((STW$l_addr_16_old$c_l_addr_none(lc, __loc0, VAR, __loc1, VAR, __loc2 = 0, VAL, SP, VAL)) || true)) {
		vector<int> params(1); params[0] = locals; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("gcc_frameless",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::param_reloc1(CSR& csr, ADDRESS& lc, int& libstub, int& locals)
{
	ADDRESS __save = lc;
	int __loc0;
	int __loc1;
	int __loc2;
	if (
	FSTDS$s_addr_im_r$c_s_addr_ma(lc, __loc0 = 7, VAL, __loc1 = 8, VAL, __loc2 = 0, VAL, SP, VAL) && 
	LDWS$s_addr_im_r$c_s_addr_notm(lc, __loc0 = -4, VAL, __loc1 = 0, VAL, SP, VAL, __loc2 = 24, VAL) && 
	LDWS$s_addr_im_r$c_s_addr_mb(lc, __loc0 = -8, VAL, __loc1 = 0, VAL, SP, VAL, __loc2 = 23, VAL) && 
	BL$c_br_null(lc, libstub, VAR, __loc0 = 0, VAL)) {
		vector<int> params(2); params[0] = libstub; params[1] = locals; 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("param_reloc1",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::gcc_unframe(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	int __loc0;
	int __loc1;
	int __loc2;
	int __loc3;
	if (
	((LDW$l_addr_16_old$c_l_addr_none(lc, __loc0 = -20, VAL, __loc1 = 0, VAL, __loc2 = 3, VAL, RP, VAL)) || true) && 
	((LDW$l_addr_16_old$c_l_addr_none(lc, __loc0 = 8, VAL, __loc1 = 0, VAL, __loc2 = 3, VAL, __loc3 = 4, VAL)) || true) && 
	LDO(lc, __loc0, VAR, __loc1 = 3, VAL, SP, VAL) && 
	LDWM$l_addr_16_old$c_l_addr_none(lc, __loc0, VAR, __loc1 = 0, VAL, SP, VAL, __loc2 = 3, VAL) && 
	BV$c_br_null(lc, R0, VAL, RP, VAL)) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("gcc_unframe",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::gcc_unframeless1(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	int __loc0;
	int __loc1;
	int __loc2;
	if (
	((LDW$l_addr_16_old$c_l_addr_none(lc, __loc0, VAR, __loc1 = 0, VAL, SP, VAL, RP, VAL)) || true) && 
	((LDW$l_addr_16_old$c_l_addr_none(lc, __loc0, VAR, __loc1 = 0, VAL, SP, VAL, __loc2, VAR)) || true) && 
	BV$c_br_nnull(lc, R0, VAL, RP, VAL) && 
	LDWM$l_addr_16_old$c_l_addr_none(lc, __loc0, VAR, __loc1 = 0, VAL, SP, VAL, __loc2, VAR)) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("gcc_unframeless1",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::gcc_unframeless2(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	int __loc0;
	int __loc1;
	int __loc2;
	if (
	((LDW$l_addr_16_old$c_l_addr_none(lc, __loc0, VAR, __loc1 = 0, VAL, SP, VAL, __loc2 = 4, VAL)) || true) && 
	LDW$l_addr_16_old$c_l_addr_none(lc, __loc0, VAR, __loc1 = 0, VAL, SP, VAL, __loc2 = 3, VAL) && 
	BV$c_br_nnull(lc, R0, VAL, RP, VAL) && 
	LDO(lc, __loc0, VAR, SP, VAL, SP, VAL)) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("gcc_unframeless2",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::bare_ret(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	if (
	BV$c_br_nnull(lc, R0, VAL, RP, VAL)) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("bare_ret",params);
	} else {
		lc = __save;
		return NULL;
	}
}
Logue* InstructionPatterns::bare_ret_anulled(CSR& csr, ADDRESS& lc)
{
	ADDRESS __save = lc;
	if (
	BV$c_br_null(lc, R0, VAL, RP, VAL)) {
		vector<int> params(0); 
		if (__save == lc) return NULL;
		return csr.instantiateLogue("bare_ret_anulled",params);
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
		params.push_back("locals");
		theSemTable.addItem("locals");
		this->newLogue("gcc_frame","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("locals");
		theSemTable.addItem("locals");
		this->newLogue("gcc_frameless","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		params.push_back("libstub");
		theSemTable.addItem("libstub");
		params.push_back("locals");
		theSemTable.addItem("locals");
		this->newLogue("param_reloc1","CALLEE_PROLOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("gcc_unframe","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("gcc_unframeless1","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("gcc_unframeless2","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("bare_ret","CALLEE_EPILOGUE",params);
	}
	{
		list<string> params;
		this->newLogue("bare_ret_anulled","CALLEE_EPILOGUE",params);
	}
}


