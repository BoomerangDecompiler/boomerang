#ifndef __hppa_pat_H__
#define __hppa_pat_H__
class InstructionPatterns {
private:
	static int R0;
	static int RP;
	static int SP;
	static int R31;
public:
	static Logue* std_call(CSR& csr, ADDRESS& lc, int& addr);
	static Logue* gcc_frame(CSR& csr, ADDRESS& lc, int& locals);
	static Logue* gcc_frameless(CSR& csr, ADDRESS& lc, int& locals);
	static Logue* param_reloc1(CSR& csr, ADDRESS& lc, int& libstub, int& locals);
	static Logue* gcc_unframe(CSR& csr, ADDRESS& lc);
	static Logue* gcc_unframeless1(CSR& csr, ADDRESS& lc);
	static Logue* gcc_unframeless2(CSR& csr, ADDRESS& lc);
	static Logue* bare_ret(CSR& csr, ADDRESS& lc);
	static Logue* bare_ret_anulled(CSR& csr, ADDRESS& lc);
private:
	static bool BL$c_br_nnull(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool BL$c_br_null(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool BV$c_br_nnull(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool BV$c_br_null(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool FSTDS$s_addr_im_r$c_s_addr_ma(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR);
	static bool LDO(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool LDW$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR);
	static bool LDWM$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR);
	static bool LDWS$s_addr_im_r$c_s_addr_mb(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR);
	static bool LDWS$s_addr_im_r$c_s_addr_notm(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR);
	static bool OR$c_arith_w$c_c_nonneg(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR);
	static bool STW$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR);
	static bool STWM$l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR, int& d, bool d_isVAR);
	static bool c_arith_w$c_c_nonneg(ADDRESS& lc, int& a, bool a_isVAR);
	static bool c_br_nnull(ADDRESS& lc);
	static bool c_br_null(ADDRESS& lc);
	static bool c_c_nonneg(ADDRESS& lc);
	static bool c_l_addr_none(ADDRESS& lc);
	static bool c_s_addr_ma(ADDRESS& lc);
	static bool c_s_addr_mb(ADDRESS& lc);
	static bool c_s_addr_notm(ADDRESS& lc);
	static bool l_addr_16_old$c_l_addr_none(ADDRESS& lc, int& a, bool a_isVAR);
	static bool s_addr_im_r$c_s_addr_ma(ADDRESS& lc, int& a, bool a_isVAR);
	static bool s_addr_im_r$c_s_addr_mb(ADDRESS& lc, int& a, bool a_isVAR);
	static bool s_addr_im_r$c_s_addr_notm(ADDRESS& lc, int& a, bool a_isVAR);
};
#endif
