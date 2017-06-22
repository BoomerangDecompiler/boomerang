#pragma once

#include "boomerang/include/frontend.h" // In case included bare, e.g. ProcTest.cpp

class Instruction;

// Class PentiumFrontEnd: derived from FrontEnd, with source machine specific
// behaviour

class PentiumDecoder;

class PentiumFrontEnd : public FrontEnd
{
public:

	/*
	 * Constructor. Takes some parameters to save passing these around a lot
	 */
	PentiumFrontEnd(IFileLoader *pLoader, Prog *prog, BinaryFileFactory *pbff);

	virtual ~PentiumFrontEnd();

	virtual Platform getFrontEndId() const override { return PLAT_PENTIUM; }

	virtual bool processProc(ADDRESS uAddr, UserProc *pProc, QTextStream& os, bool frag = false, bool spec = false) override;

	virtual std::vector<SharedExp>& getDefaultParams() override;

	virtual std::vector<SharedExp>& getDefaultReturns() override;

	virtual ADDRESS getMainEntryPoint(bool& gotMain) override;

private:

	/*
	 * Process an F(n)STSW instruction.
	 */
	bool processStsw(std::list<RTL *>::iterator& rit, std::list<RTL *> *pRtls, BasicBlock *pBB, Cfg *pCfg);

	/*
	 * Emit a set instruction.
	 */
	void emitSet(std::list<RTL *> *pRtls, std::list<RTL *>::iterator& itRtl, ADDRESS uAddr, SharedExp pLHS, SharedExp cond);

	/*
	 * Handle the case of being in state 23 and encountering a set instruction.
	 */
	void State25(SharedExp pLHS, SharedExp pRHS, std::list<RTL *> *pRtls, std::list<RTL *>::iterator& rit, ADDRESS uAddr);

	int idPF; // Parity flag

	void processFloatCode(Cfg *pCfg);

	void processFloatCode(BasicBlock *pBB, int& tos, Cfg *pCfg);
	void processStringInst(UserProc *proc);
	void processOverlapped(UserProc *proc);

	/*
	 * Check a HLCall for a helper function, and replace with appropriate
	 *    semantics if possible
	 */
	bool helperFunc(ADDRESS dest, ADDRESS addr, std::list<RTL *> *lrtl) override;

	bool isStoreFsw(Instruction *s);
	bool isDecAh(RTL *r);
	bool isSetX(Instruction *s);
	bool isAssignFromTern(Instruction *s);
	void bumpRegisterAll(SharedExp e, int min, int max, int delta, int mask);
	unsigned fetch4(unsigned char *ptr);
	bool decodeSpecial(ADDRESS pc, DecodeResult& r);
	bool decodeSpecial_out(ADDRESS pc, DecodeResult& r);
	bool decodeSpecial_invalid(ADDRESS pc, DecodeResult& r);

protected:
	virtual DecodeResult& decodeInstruction(ADDRESS pc) override;
	virtual void extraProcessCall(CallStatement *call, std::list<RTL *> *BB_rtls) override;
};
