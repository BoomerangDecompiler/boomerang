#ifndef __mc68k_pat_H__
#define __mc68k_pat_H__
class InstructionPatterns {
private:
	static int SP;
	static int FP;
public:
	static Logue* std_call(CSR& csr, ADDRESS& lc, int& addr);
	static Logue* near_call(CSR& csr, ADDRESS& lc, int& addr);
	static Logue* pea_add_rts(CSR& csr, ADDRESS& lc, int& d32);
	static Logue* pea_pea_add_rts(CSR& csr, ADDRESS& lc, int& d32);
	static Logue* trap_syscall(CSR& csr, ADDRESS& lc, int& d16);
	static Logue* std_link(CSR& csr, ADDRESS& lc, int& locals);
	static Logue* link_save(CSR& csr, ADDRESS& lc, int& locals, int& d16);
	static Logue* link_save1(CSR& csr, ADDRESS& lc, int& locals, int& reg);
	static Logue* push_lea(CSR& csr, ADDRESS& lc, int& locals, int& reg);
	static Logue* bare_ret(CSR& csr, ADDRESS& lc);
	static Logue* std_ret(CSR& csr, ADDRESS& lc);
	static Logue* rest_ret(CSR& csr, ADDRESS& lc, int& d16);
	static Logue* rest1_ret(CSR& csr, ADDRESS& lc, int& reg);
	static Logue* pop_ret(CSR& csr, ADDRESS& lc, int& reg);
	static Logue* clear_stack(CSR& csr, ADDRESS& lc, int& n);
private:
	static bool Aline(ADDRESS& lc, int& a, bool a_isVAR);
	static bool addaw_d16(ADDRESS& lc, int& a, bool a_isVAR);
	static bool addil$daIndirect(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool bsr(ADDRESS& lc, int& a, bool a_isVAR);
	static bool call_(ADDRESS& lc, int& a, bool a_isVAR);
	static bool daIndirect(ADDRESS& lc, int& a, bool a_isVAR);
	static bool daPostInc(ADDRESS& lc, int& a, bool a_isVAR);
	static bool daPreDec(ADDRESS& lc, int& a, bool a_isVAR);
	static bool leaSpSp(ADDRESS& lc, int& a, bool a_isVAR);
	static bool link(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool movemrl$daPostInc(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool moverml$daPreDec(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool peaPcDisp(ADDRESS& lc, int& a, bool a_isVAR);
	static bool popreg(ADDRESS& lc, int& a, bool a_isVAR);
	static bool pushreg(ADDRESS& lc, int& a, bool a_isVAR);
	static bool rts(ADDRESS& lc);
	static bool trap(ADDRESS& lc, int& a, bool a_isVAR);
	static bool unlk(ADDRESS& lc, int& a, bool a_isVAR);
};
#endif
