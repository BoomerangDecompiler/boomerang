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
	match [nextPC] lc to
	| Aline(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::addaw_d16(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| addaw_d16(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::addil$daIndirect(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| addil(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!daIndirect(_b, b, b_isVAR)) return false;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::bsr(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| bsr(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::call_(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| call_(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::daIndirect(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| daIndirect(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::daPostInc(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| daPostInc(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::daPreDec(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| daPreDec(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::leaSpSp(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| leaSpSp(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::link(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| link(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::movemrl$daPostInc(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| movemrl(_a, _b) =>
		if (!daPostInc(_a, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::moverml$daPreDec(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| moverml(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!daPreDec(_b, b, b_isVAR)) return false;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::peaPcDisp(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| peaPcDisp(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::popreg(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| popreg(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::pushreg(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| pushreg(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::rts(ADDRESS& lc) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| rts() =>
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::trap(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| trap(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::unlk(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| unlk(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
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
