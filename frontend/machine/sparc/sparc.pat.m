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
	match [nextPC] lc to
	| ADD(_a, _b, _c) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!imode(_b, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_c != c) return false; else c = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::ADD$rmode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| ADD(_a, _b, _c) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!rmode(_b, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_c != c) return false; else c = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::JMPL$dispA(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| JMPL(_a, _b) =>
		if (!dispA(_a, a, a_isVAR, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_b != c) return false; else c = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::JMPL$indirectA(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| JMPL(_a, _b) =>
		if (!indirectA(_a, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::OR$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| OR(_a, _b, _c) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!imode(_b, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_c != c) return false; else c = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::RESTORE$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| RESTORE(_a, _b, _c) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!imode(_b, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_c != c) return false; else c = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::RESTORE$rmode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| RESTORE(_a, _b, _c) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!rmode(_b, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_c != c) return false; else c = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::SAVE$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| SAVE(_a, _b, _c) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!imode(_b, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_c != c) return false; else c = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::SAVE$rmode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| SAVE(_a, _b, _c) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!rmode(_b, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_c != c) return false; else c = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::SUB$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| SUB(_a, _b, _c) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!imode(_b, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_c != c) return false; else c = _c;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::UNIMP(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| UNIMP(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::call__(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| call__(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::dispA(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	match lc to
	| dispA(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::imode(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| imode(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::indirectA(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| indirectA(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::mov_(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| mov_(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::restore_(ADDRESS& lc) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| restore_() =>
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::ret(ADDRESS& lc) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| ret() =>
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::retl(ADDRESS& lc) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| retl() =>
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::rmode(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| rmode(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::sethi(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| sethi(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
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
