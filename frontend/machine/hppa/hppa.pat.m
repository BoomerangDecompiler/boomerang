/*==============================================
 * FILE:      hppa.pat.m
 * OVERVIEW:  Generated file; do not edit
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
	match [nextPC] lc to
	| BL(_a, _b, _c) =>
		if (!c_br_nnull(_a)) return false;
		if (!a_isVAR && (int)_b != a) return false; else a = _b;
		if (!b_isVAR && (int)_c != b) return false; else b = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::BL$c_br_null(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| BL(_a, _b, _c) =>
		if (!c_br_null(_a)) return false;
		if (!a_isVAR && (int)_b != a) return false; else a = _b;
		if (!b_isVAR && (int)_c != b) return false; else b = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::BV$c_br_nnull(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| BV(_a, _b, _c) =>
		if (!c_br_nnull(_a)) return false;
		if (!a_isVAR && (int)_b != a) return false; else a = _b;
		if (!b_isVAR && (int)_c != b) return false; else b = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::BV$c_br_null(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| BV(_a, _b, _c) =>
		if (!c_br_null(_a)) return false;
		if (!a_isVAR && (int)_b != a) return false; else a = _b;
		if (!b_isVAR && (int)_c != b) return false; else b = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::FSTDS$s_addr_im_r$c_s_addr_ma(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| FSTDS(_a, _b, _c, _d, _e) =>
		if (!c_s_addr_ma(_a)) return false;
		if (!a_isVAR && (int)_b != a) return false; else a = _b;
		if (!s_addr_im_r$c_s_addr_ma(_c, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_d != c) return false; else c = _d;
		if (!d_isVAR && (int)_e != d) return false; else d = _e;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::LDO(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| LDO(_a, _b, _c) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		if (!c_isVAR && (int)_c != c) return false; else c = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::LDW$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| LDW(_a, _b, _c, _d, _e) =>
		if (!c_l_addr_none(_a)) return false;
		if (!l_addr_16_old$c_l_addr_none(_b, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_c != b) return false; else b = _c;
		if (!c_isVAR && (int)_d != c) return false; else c = _d;
		if (!d_isVAR && (int)_e != d) return false; else d = _e;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::LDWM$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| LDWM(_a, _b, _c, _d, _e) =>
		if (!c_l_addr_none(_a)) return false;
		if (!l_addr_16_old$c_l_addr_none(_b, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_c != b) return false; else b = _c;
		if (!c_isVAR && (int)_d != c) return false; else c = _d;
		if (!d_isVAR && (int)_e != d) return false; else d = _e;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::LDWS$s_addr_im_r$c_s_addr_mb(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| LDWS(_a, _b, _c, _d, _e) =>
		if (!c_s_addr_mb(_a)) return false;
		if (!s_addr_im_r$c_s_addr_mb(_b, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_c != b) return false; else b = _c;
		if (!c_isVAR && (int)_d != c) return false; else c = _d;
		if (!d_isVAR && (int)_e != d) return false; else d = _e;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::LDWS$s_addr_im_r$c_s_addr_notm(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| LDWS(_a, _b, _c, _d, _e) =>
		if (!c_s_addr_notm(_a)) return false;
		if (!s_addr_im_r$c_s_addr_notm(_b, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_c != b) return false; else b = _c;
		if (!c_isVAR && (int)_d != c) return false; else c = _d;
		if (!d_isVAR && (int)_e != d) return false; else d = _e;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::OR$c_arith_w$c_c_nonneg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| OR(_a, _b, _c, _d) =>
		if (!c_arith_w$c_c_nonneg(_a, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		if (!c_isVAR && (int)_c != c) return false; else c = _c;
		if (!d_isVAR && (int)_d != d) return false; else d = _d;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::STW$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| STW(_a, _b, _c, _d, _e) =>
		if (!c_l_addr_none(_a)) return false;
		if (!a_isVAR && (int)_b != a) return false; else a = _b;
		if (!l_addr_16_old$c_l_addr_none(_c, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_d != c) return false; else c = _d;
		if (!d_isVAR && (int)_e != d) return false; else d = _e;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::STWM$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| STWM(_a, _b, _c, _d, _e) =>
		if (!c_l_addr_none(_a)) return false;
		if (!a_isVAR && (int)_b != a) return false; else a = _b;
		if (!l_addr_16_old$c_l_addr_none(_c, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_d != c) return false; else c = _d;
		if (!d_isVAR && (int)_e != d) return false; else d = _e;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::c_arith_w$c_c_nonneg(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| c_arith_w(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!c_c_nonneg(_b)) return false;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::c_br_nnull(ADDRESS& lc) {
	match lc to
	| c_br_nnull() =>
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::c_br_null(ADDRESS& lc) {
	match lc to
	| c_br_null() =>
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::c_c_nonneg(ADDRESS& lc) {
	match lc to
	| c_c_nonneg() =>
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::c_l_addr_none(ADDRESS& lc) {
	match lc to
	| c_l_addr_none() =>
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::c_s_addr_ma(ADDRESS& lc) {
	match lc to
	| c_s_addr_ma() =>
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::c_s_addr_mb(ADDRESS& lc) {
	match lc to
	| c_s_addr_mb() =>
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::c_s_addr_notm(ADDRESS& lc) {
	match lc to
	| c_s_addr_notm() =>
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| l_addr_16_old(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::s_addr_im_r$c_s_addr_ma(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| s_addr_im_r(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::s_addr_im_r$c_s_addr_mb(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| s_addr_im_r(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::s_addr_im_r$c_s_addr_notm(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| s_addr_im_r(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
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
