#ifndef __sparc_pat_H__
#define __sparc_pat_H__
class InstructionPatterns {
private:
	static int SP;
	static int FP;
	static int o0;
	static int i0;
	static int i7;
	static int o7;
	static int g0;
public:
	static Logue* std_call(CSR& csr, ADDRESS& lc, int& addr);
	static Logue* struct_call(CSR& csr, ADDRESS& lc, int& addr, int& imm22);
	static Logue* call_rst_ui_reg(CSR& csr, ADDRESS& lc, int& addr, int& imm22, int& rs1, int& rs2, int& rd);
	static Logue* call_rst_ui_imm(CSR& csr, ADDRESS& lc, int& addr, int& imm22, int& rs1, int& imm, int& rd);
	static Logue* call_restore_reg(CSR& csr, ADDRESS& lc, int& addr, int& rs1, int& rs2, int& rd);
	static Logue* call_restore_imm(CSR& csr, ADDRESS& lc, int& addr, int& rs1, int& imm, int& rd);
	static Logue* move_call_move(CSR& csr, ADDRESS& lc, int& addr, int& rd);
	static Logue* move_x_call_move(CSR& csr, ADDRESS& lc, int& addr, int& rd);
	static Logue* call_add(CSR& csr, ADDRESS& lc, int& addr, int& imm);
	static Logue* jmp_restore_reg(CSR& csr, ADDRESS& lc, int& rs1j, int& rdj, int& rs1, int& rs2, int& rd);
	static Logue* jmp_restore_imm(CSR& csr, ADDRESS& lc, int& rs1j, int& rdj, int& rs1, int& imm, int& rd);
	static Logue* new_reg_win(CSR& csr, ADDRESS& lc, int& locals);
	static Logue* same_reg_win(CSR& csr, ADDRESS& lc, int& locals);
	static Logue* new_reg_win_large(CSR& csr, ADDRESS& lc, int& hiVal, int& loVal, int& reg);
	static Logue* same_reg_win_large(CSR& csr, ADDRESS& lc, int& hiVal, int& loVal, int& reg);
	static Logue* none(CSR& csr, ADDRESS& lc);
	static Logue* std_ret(CSR& csr, ADDRESS& lc);
	static Logue* ret_reg_val(CSR& csr, ADDRESS& lc, int& rs1, int& rs2);
	static Logue* ret_imm_val(CSR& csr, ADDRESS& lc, int& rs1, int& imm);
	static Logue* leaf_ret(CSR& csr, ADDRESS& lc);
	static Logue* ret_struct4(CSR& csr, ADDRESS& lc);
private:
	static bool ADD$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool ADD$rmode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool JMPL$dispA(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool JMPL$indirectA(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool OR$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool RESTORE$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool RESTORE$rmode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool SAVE$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool SAVE$rmode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool SUB$imode(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR, int& c, bool c_isVAR);
	static bool UNIMP(ADDRESS& lc, int& a, bool a_isVAR);
	static bool call__(ADDRESS& lc, int& a, bool a_isVAR);
	static bool dispA(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool imode(ADDRESS& lc, int& a, bool a_isVAR);
	static bool indirectA(ADDRESS& lc, int& a, bool a_isVAR);
	static bool mov_(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
	static bool restore_(ADDRESS& lc);
	static bool ret(ADDRESS& lc);
	static bool retl(ADDRESS& lc);
	static bool rmode(ADDRESS& lc, int& a, bool a_isVAR);
	static bool sethi(ADDRESS& lc, int& a, bool a_isVAR, int& b, bool b_isVAR);
};
#endif
