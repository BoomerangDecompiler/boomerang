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
	match [nextPC] lc to
	| ADDid(_a, _b) =>
		if (!Reg(_a, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::ADDiodb$E$Base8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| ADDiodb(_a, _b) =>
		if (!E$Base8(_a, a, a_isVAR, b, b_isVAR)) return false;
		if (!c_isVAR && (int)_b != c) return false; else c = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::ADDiodb$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| ADDiodb(_a, _b) =>
		if (!Reg(_a, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::Base(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| Base(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::Base8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	match lc to
	| Base8(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::CALL$Jvod(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| CALL.Jvod(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::CLD(ADDRESS& lc) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| CLD() =>
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	match lc to
	| Disp8(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::E$Base(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| E(_a) =>
		if (!Base(_a, a, a_isVAR)) return false;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::E$Base8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	match lc to
	| E(_a) =>
		if (!Base8(_a, a, a_isVAR, b, b_isVAR)) return false;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::E$Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	match lc to
	| E(_a) =>
		if (!Disp8(_a, a, a_isVAR, b, b_isVAR)) return false;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::JMP$Jvod(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| JMP.Jvod(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::LEAVE(ADDRESS& lc) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| LEAVE() =>
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::LEAod$Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| LEAod(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::MOVSB(ADDRESS& lc) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| MOVSB() =>
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::MOVSvow(ADDRESS& lc) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| MOVSvow() =>
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::MOVid(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| MOVid(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::MOVmrod$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| MOVmrod(_a, _b) =>
		if (!Reg(_a, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::MOVrmod$E$Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| MOVrmod(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!E$Disp8(_b, b, b_isVAR, c, c_isVAR)) return false;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::MOVrmod$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| MOVrmod(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!Reg(_b, b, b_isVAR)) return false;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::POPod(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| POPod(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::PUSHod(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| PUSHod(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::REP$MOVSvod(ADDRESS& lc) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| REP.MOVSvod() =>
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::REPNE$SCASB(ADDRESS& lc) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| REPNE.SCASB() =>
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::RET(ADDRESS& lc) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| RET() =>
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::RET$Iw(ADDRESS& lc, int& a, bool a_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| RET.Iw(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::Reg(ADDRESS& lc, int& a, bool a_isVAR) {
	match lc to
	| Reg(_a) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::SUBid$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| SUBid(_a, _b) =>
		if (!Reg(_a, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::SUBiodb$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| SUBiodb(_a, _b) =>
		if (!Reg(_a, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::XCHG$Ev$Gvod$E$Base(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| XCHG.Ev.Gvod(_a, _b) =>
		if (!E$Base(_a, a, a_isVAR)) return false;
		if (!b_isVAR && (int)_b != b) return false; else b = _b;
		lc = nextPC;
		return true;
	else
		return false;
	endmatch
}
bool InstructionPatterns::XORrmod$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR) {
	ADDRESS nextPC;
	match [nextPC] lc to
	| XORrmod(_a, _b) =>
		if (!a_isVAR && (int)_a != a) return false; else a = _a;
		if (!Reg(_b, b, b_isVAR)) return false;
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
