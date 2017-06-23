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
	virtual bool processProc(ADDRESS uAddr, UserProc *pProc, QTextStream& os, bool frag = false, bool spec = false) override;

	/// @copydoc IFrontEnd::getDefaultParams
	virtual std::vector<SharedExp>& getDefaultParams() override;

	/// @copydoc IFrontEnd::getDefaultReturns
	virtual std::vector<SharedExp>& getDefaultReturns() override;

	/// @copydoc IFrontEnd::getMainEntryPoint
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
	bool isHelperFunc(ADDRESS dest, ADDRESS addr, std::list<RTL *> *lrtl) override;

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
