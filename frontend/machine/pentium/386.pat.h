#ifndef __386_pat_H__
#define __386_pat_H__
class InstructionPatterns {
private:
	static int EAX;
	static int ECX;
	static int EDX;
	static int EBX;
	static int ESP;
	static int EBP;
	static int ESI;
	static int EDI;
public:
	static Logue* std_call(CSR& csr, ADDRESS& lc, int& addr);
	static Logue* pat_strlen(CSR& csr, ADDRESS& lc);
	static Logue* pat_memcpy_00(CSR& csr, ADDRESS& lc, int& len);
	static Logue* pat_memcpy_01(CSR& csr, ADDRESS& lc, int& len);
	static Logue* pat_memcpy_10(CSR& csr, ADDRESS& lc, int& len);
	static Logue* pat_memcpy_11(CSR& csr, ADDRESS& lc, int& len);
	static Logue* pat_this_thunk(CSR& csr, ADDRESS& lc, int& off, int& dest);
static int iterhlp0(ADDRESS& lc, int& regs);
	static Logue* std_entry(CSR& csr, ADDRESS& lc, int& locals, int& regs);
	static Logue* struct_ptr(CSR& csr, ADDRESS& lc, int& locals, int& regs);
static int iterhlp1(ADDRESS& lc, int& regs);
	static Logue* frameless_pro(CSR& csr, ADDRESS& lc, int& locals, int& regs);
	static Logue* none(CSR& csr, ADDRESS& lc);
static int iterhlp2(ADDRESS& lc, int& __loop, int& __loc0);
static int iterhlp3(ADDRESS& lc, int& __loop, int& __loc0);
	static Logue* std_ret(CSR& csr, ADDRESS& lc);
static int iterhlp4(ADDRESS& lc, int& __loop);
	static Logue* frameless_epi(CSR& csr, ADDRESS& lc, int& n);
static int iterhlp5(ADDRESS& lc, int& n);
	static Logue* clear_stack(CSR& csr, ADDRESS& lc, int& n);
private:
	static bool ADDid$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool ADDiodb$E$Base8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool ADDiodb$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool Base(ADDRESS& lc, int& a, bool a_isVAR);
	static bool Base8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool CALL$Jvod(ADDRESS& lc, int& a, bool a_isVAR);
	static bool CLD(ADDRESS& lc);
	static bool Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool E$Base(ADDRESS& lc, int& a, bool a_isVAR);
	static bool E$Base8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool E$Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool JMP$Jvod(ADDRESS& lc, int& a, bool a_isVAR);
	static bool LEAVE(ADDRESS& lc);
	static bool LEAod$Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool MOVSB(ADDRESS& lc);
	static bool MOVSvow(ADDRESS& lc);
	static bool MOVid(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool MOVmrod$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool MOVrmod$E$Disp8(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool MOVrmod$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool POPod(ADDRESS& lc, int& a, bool a_isVAR);
	static bool PUSHod(ADDRESS& lc, int& a, bool a_isVAR);
	static bool REP$MOVSvod(ADDRESS& lc);
	static bool REPNE$SCASB(ADDRESS& lc);
	static bool RET(ADDRESS& lc);
	static bool RET$Iw(ADDRESS& lc, int& a, bool a_isVAR);
	static bool Reg(ADDRESS& lc, int& a, bool a_isVAR);
	static bool SUBid$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool SUBiodb$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool XCHG$Ev$Gvod$E$Base(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool XORrmod$Reg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
};
#endif
