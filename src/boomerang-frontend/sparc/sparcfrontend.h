#pragma once

// Class SparcFrontEnd: derived from FrontEnd, with source machine specific
// behaviour

#include <set>

#include "boomerang-frontend/frontend.h"
#include "boomerang-frontend/decoder.h"

#include "boomerang/db/exp.h"           // Ugh... just for enum OPER


class IFrontEnd;
class SparcDecoder;
class CallStatement;

struct DecodeResult;


class SparcFrontEnd : public IFrontEnd
{
public:
	/// @copydoc IFrontEnd::IFrontEnd
	SparcFrontEnd(IFileLoader *p_BF, Prog *prog, BinaryFileFactory *bff);
	virtual ~SparcFrontEnd();

	/// @copydoc IFrontEnd::getType
	virtual Platform getType() const override { return Platform::SPARC; }

	/**
	 * @copydoc IFrontEnd::processProc
	 * 
	 * This overrides the base class processProc to do source machine
	 * specific things (but often calls the base class to do most of the
	 * work. Sparc is an exception)
	 */
	virtual bool processProc(ADDRESS uAddr, UserProc *proc, QTextStream& os, bool fragment = false, bool spec = false) override;

	/// @copydoc IFrontEnd::getDefaultParams
	virtual std::vector<SharedExp>& getDefaultParams() override;

	/// @copydoc IFrontEnd::getDefaultReturns
	virtual std::vector<SharedExp>& getDefaultReturns() override;

	/// @copydoc IFrontEnd::getMainEntryPoint
	virtual ADDRESS getMainEntryPoint(bool& gotMain) override;

private:
	void warnDCTcouple(ADDRESS uAt, ADDRESS uDest);
	bool optimise_DelayCopy(ADDRESS src, ADDRESS dest, ptrdiff_t delta, ADDRESS uUpper);
	BasicBlock *optimise_CallReturn(CallStatement *call, RTL *rtl, RTL *delay, UserProc *pProc);

	void handleBranch(ADDRESS dest, ADDRESS hiAddress, BasicBlock *& newBB, Cfg *cfg, TargetQueue& tq);
	void handleCall(UserProc *proc, ADDRESS dest, BasicBlock *callBB, Cfg *cfg, ADDRESS address, int offset = 0);

	void case_unhandled_stub(ADDRESS addr);

	bool case_CALL(ADDRESS& address, DecodeResult& inst, DecodeResult& delay_inst, std::list<RTL *> *& BB_rtls,
				   UserProc *proc, std::list<CallStatement *>& callList, QTextStream& os, bool isPattern = false);

	void case_SD(ADDRESS& address, ptrdiff_t delta, ADDRESS hiAddress, DecodeResult& inst, DecodeResult& delay_inst,
				 std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq, QTextStream& os);

	bool case_DD(ADDRESS& address, ptrdiff_t delta, DecodeResult& inst, DecodeResult& delay_inst,
				 std::list<RTL *> *& BB_rtls, TargetQueue& tq, UserProc *proc, std::list<CallStatement *>& callList);

	bool case_SCD(ADDRESS& address, ptrdiff_t delta, ADDRESS hiAddress, DecodeResult& inst, DecodeResult& delay_inst,
				  std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq);

	bool case_SCDAN(ADDRESS& address, ptrdiff_t delta, ADDRESS hiAddress, DecodeResult& inst, DecodeResult& delay_inst,
					std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq);

	void emitNop(std::list<RTL *> *pRtls, ADDRESS uAddr);
	void emitCopyPC(std::list<RTL *> *pRtls, ADDRESS uAddr);
	unsigned fetch4(unsigned char *ptr);
	void appendAssignment(const SharedExp& lhs, const SharedExp& rhs, SharedType type, ADDRESS addr, std::list<RTL *> *lrtl);
	void quadOperation(ADDRESS addr, std::list<RTL *> *lrtl, OPER op);

	bool isHelperFunc(ADDRESS dest, ADDRESS addr, std::list<RTL *> *lrtl) override;
	void gen32op32gives64(OPER op, std::list<RTL *> *lrtl, ADDRESS addr);
	bool helperFuncLong(ADDRESS dest, ADDRESS addr, std::list<RTL *> *lrtl, QString& name);

	// void    setReturnLocations(CalleeEpilogue* epilogue, int iReg);

	// This struct represents a single nop instruction. Used as a substitute delay slot instruction
	DecodeResult nop_inst;
	IBinarySymbolTable *SymbolTable;
};
