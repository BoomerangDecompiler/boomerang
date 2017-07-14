#pragma once

#include "boomerang/frontend/frontend.h"

class Instruction;
class PentiumDecoder;

// Class PentiumFrontEnd: derived from FrontEnd, with source machine specific
// behaviour
class PentiumFrontEnd : public IFrontEnd
{
public:
	/// @copydoc IFrontEnd::IFrontEnd
	PentiumFrontEnd(IFileLoader *pLoader, Prog *prog, BinaryFileFactory *pbff);

	/// @copydoc IFrontEnd::~IFrontEnd
	virtual ~PentiumFrontEnd();

	/// @copydoc IFrontEnd::getFrontEndId
	virtual Platform getType() const override { return Platform::PENTIUM; }

	/// @copydoc IFrontEnd::processProc
	virtual bool processProc(Address uAddr, UserProc *pProc, QTextStream& os, bool frag = false, bool spec = false) override;

	/// @copydoc IFrontEnd::getDefaultParams
	virtual std::vector<SharedExp>& getDefaultParams() override;

	/// @copydoc IFrontEnd::getDefaultReturns
	virtual std::vector<SharedExp>& getDefaultReturns() override;

	/// @copydoc IFrontEnd::getMainEntryPoint
	virtual Address getMainEntryPoint(bool& gotMain) override;

private:
	/*
	 * Process an F(n)STSW instruction.
	 */
	bool processStsw(std::list<RTL *>::iterator& rit, std::list<RTL *> *pRtls, BasicBlock *pBB, Cfg *pCfg);

	/*
	 * Emit a set instruction.
	 */
	void emitSet(std::list<RTL *> *pRtls, std::list<RTL *>::iterator& itRtl, Address uAddr, SharedExp pLHS, SharedExp cond);

	/*
	 * Handle the case of being in state 23 and encountering a set instruction.
	 */
	void State25(SharedExp pLHS, SharedExp pRHS, std::list<RTL *> *pRtls, std::list<RTL *>::iterator& rit, Address uAddr);

	int idPF; // Parity flag

	void processFloatCode(Cfg *pCfg);

	/***************************************************************************/ /**
	* \brief Process a basic block, and all its successors, for floating point code.
	*  Remove FPUSH/FPOP, instead decrementing or incrementing respectively the tos value to be used from
	*  here down.
	* \note tos has to be a parameter, not a global, to get the right value at any point in
	*  the call tree
	* \param pBB pointer to the current BB
	* \param tos reference to the value of the "top of stack" pointer currently. Starts at zero, and is
	*        decremented to 7 with the first load, so r[39] should be used first, then r[38] etc. However, it is
	*        reset to 0 for calls, so that if a function returns a float, then it will always appear in r[32]
	* \param pCfg passed to processFloatCode
	******************************************************************************/
	void processFloatCode(BasicBlock *pBB, int& tos, Cfg *pCfg);
	void processStringInst(UserProc *proc);
	void processOverlapped(UserProc *proc);

	/***************************************************************************/ /**
	* \brief Checks for pentium specific helper functions like __xtol which have specific sematics.
	*
	* \note This needs to be handled in a resourcable way.
	*
	* \param dest - the native destination of this call
	* \param addr - the native address of this call instruction
	* \param lrtl - pointer to a list of RTL pointers for this BB
	*
	* \returns true if a helper function is converted; false otherwise
	******************************************************************************/
	bool isHelperFunc(Address dest, Address addr, std::list<RTL *> *lrtl) override;

	bool isStoreFsw(Instruction *s);
	bool isDecAh(RTL *r);
	bool isSetX(Instruction *s);
	bool isAssignFromTern(Instruction *s);

	/***************************************************************************/ /**
	* \fn        PentiumFrontEnd::bumpRegisterAll
	* \brief        Finds a subexpression within this expression of the form
	*                      r[ int x] where min <= x <= max, and replaces it with
	*                      r[ int y] where y = min + (x - min + delta & mask)
	* \param e - Expression to modify
	* \param min - minimum register numbers before any change is considered
	* \param max - maximum register numbers before any change is considered
	* \param delta amount to bump up the register number by
	* \param mask see above
	* APPLICATION:        Used to "flatten" stack floating point arithmetic (e.g. Pentium floating point code)
	*                      If registers are not replaced "all at once" like this, there can be subtle errors from
	*                      re-replacing already replaced registers
	*
	******************************************************************************/
	void bumpRegisterAll(SharedExp e, int min, int max, int delta, int mask);
	unsigned fetch4(unsigned char *ptr);
	bool decodeSpecial(Address pc, DecodeResult& r);
	bool decodeSpecial_out(Address pc, DecodeResult& r);
	bool decodeSpecial_invalid(Address pc, DecodeResult& r);

protected:
	virtual DecodeResult& decodeInstruction(Address pc) override;
	virtual void extraProcessCall(CallStatement *call, std::list<RTL *> *BB_rtls) override;
};
