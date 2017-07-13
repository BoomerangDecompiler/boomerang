#pragma once

// Class SparcFrontEnd: derived from FrontEnd, with source machine specific
// behaviour

#include <set>

#include "boomerang/frontend/frontend.h"
#include "boomerang/frontend/decoder.h"

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
	virtual bool processProc(Address uAddr, UserProc *proc, QTextStream& os, bool fragment = false, bool spec = false) override;

	/// @copydoc IFrontEnd::getDefaultParams
	virtual std::vector<SharedExp>& getDefaultParams() override;

	/// @copydoc IFrontEnd::getDefaultReturns
	virtual std::vector<SharedExp>& getDefaultReturns() override;

	/// @copydoc IFrontEnd::getMainEntryPoint
	virtual Address getMainEntryPoint(bool& gotMain) override;

private:
	void warnDCTcouple(Address uAt, Address uDest);
	bool optimise_DelayCopy(Address src, Address dest, ptrdiff_t delta, Address uUpper);

	/***************************************************************************/ /**
	* \fn     SparcFrontEnd::optimise_CallReturn
	* \brief      Determines if the given call and delay instruction consitute a call where callee returns to
	*                    the caller's caller. That is:
	*   ProcA      |  ProcB:       |         ProcC:
	* -------------|---------------|---------------
	* ...          | ...           |        ...
	* call ProcB   | call ProcC    |        ret
	* ...          | restore       |        ...
	*
	*             The restore instruction in ProcB will effectively set %o7 to be %i7, the address to which ProcB will
	*             return. So in effect ProcC will return to ProcA at the position just after the call to ProcB. This
	*             is equivalent to ProcC returning to ProcB which then immediately returns to ProcA.
	*
	* \note               We don't set a label at the return, because we also have to force a jump at the call BB,
	*                 and in some cases we don't have the call BB as yet. So these two are up to the caller
	* \note               The name of this function is now somewhat irrelevant. The whole function is somewhat irrelevant;
	*                 it dates from the times when you would always find an actual restore in the delay slot.
	*                 With some patterns, this is no longer true.
	* \param          call - the RTL for the caller (e.g. "call ProcC" above)
	* \param          rtl - pointer to the RTL for the call instruction
	* \param          delay - the RTL for the delay instruction (e.g. "restore")
	* \returns        The basic block containing the single return instruction if this optimisation applies, nullptr
	*                 otherwise.
	******************************************************************************/
	BasicBlock *optimise_CallReturn(CallStatement *call, RTL *rtl, RTL *delay, UserProc *pProc);

	/***************************************************************************/ /**
	* \fn    SparcFrontEnd::handleBranch
	* \brief Adds the destination of a branch to the queue of address that must be decoded (if this destination
	*        has not already been visited).
	*, but newBB may be changed if the destination of the branch is in the middle of an existing
	*        BB. It will then be changed to point to a new BB beginning with the dest
	* \param newBB - the new basic block delimited by the branch instruction. May be nullptr if this block has been
	*        built before.
	* \param dest - the destination being branched to
	* \param hiAddress - the last address in the current procedure
	* \param cfg - the CFG of the current procedure
	* \param tq Object managing the target queue
	******************************************************************************/
	void handleBranch(Address dest, Address hiAddress, BasicBlock *& newBB, Cfg *cfg, TargetQueue& tq);
	void handleCall(UserProc *proc, Address dest, BasicBlock *callBB, Cfg *cfg, Address address, int offset = 0);

	/***************************************************************************/ /**
	* \fn         SparcFrontEnd::case_unhandled_stub
	* \brief         This is the stub for cases of DCTI couples that we haven't written analysis code for yet. It simply
	*                        displays an informative warning and returns.
	* \param         addr - the address of the first CTI in the couple
	*
	******************************************************************************/
	void case_unhandled_stub(Address addr);

	/***************************************************************************/ /**
	* \fn         SparcFrontEnd::case_CALL
	* \brief         Handles a call instruction
	* \param address - the native address of the call instruction
	* \param inst - the info summaries when decoding the call instruction
	* \param delay_inst - the info summaries when decoding the delay instruction
	* \param BB_rtls - the list of RTLs currently built for the BB under construction
	* \param proc - the enclosing procedure
	* \param callList - a list of pointers to CallStatements for procs yet to be processed
	* \param os - output stream for rtls
	* \param isPattern - true if the call is an idiomatic pattern (e.g. a move_call_move pattern)
	* SIDE EFFECTS:     address may change; BB_rtls may be appended to or set nullptr
	* \returns              true if next instruction is to be fetched sequentially from this one
	******************************************************************************/
	bool case_CALL(Address& address, DecodeResult& inst, DecodeResult& delay_inst, std::list<RTL *> *& BB_rtls,
				   UserProc *proc, std::list<CallStatement *>& callList, QTextStream& os, bool isPattern = false);

	/***************************************************************************/ /**
	* \fn         SparcFrontEnd::case_SD
	* \brief         Handles a non-call, static delayed (SD) instruction
	* \param address - the native address of the SD
	* \param delta - the offset of the above address from the logical address at which the procedure starts
	*                (i.e. the one given by dis)
	* \param inst - the info summaries when decoding the SD instruction
	* \param delay_inst - the info summaries when decoding the delay instruction
	* \param BB_rtls - the list of RTLs currently built for the BB under construction
	* \param cfg - the CFG of the enclosing procedure
	* \param tq - Object managing the target queue
	* \param os - output stream for rtls
	* SIDE EFFECTS:     address may change; BB_rtls may be appended to or set nullptr
	*
	******************************************************************************/
	void case_SD(Address& address, ptrdiff_t delta, Address hiAddress, DecodeResult& inst, DecodeResult& delay_inst,
				 std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq, QTextStream& os);

	bool case_DD(Address& address, ptrdiff_t delta, DecodeResult& inst, DecodeResult& delay_inst,
				 std::list<RTL *> *& BB_rtls, TargetQueue& tq, UserProc *proc, std::list<CallStatement *>& callList);

	bool case_SCD(Address& address, ptrdiff_t delta, Address hiAddress, DecodeResult& inst, DecodeResult& delay_inst,
				  std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq);

	bool case_SCDAN(Address& address, ptrdiff_t delta, Address hiAddress, DecodeResult& inst, DecodeResult& delay_inst,
					std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq);

	void emitNop(std::list<RTL *> *pRtls, Address uAddr);
	void emitCopyPC(std::list<RTL *> *pRtls, Address uAddr);
	unsigned fetch4(unsigned char *ptr);
	void appendAssignment(const SharedExp& lhs, const SharedExp& rhs, SharedType type, Address addr, std::list<RTL *> *lrtl);
	void quadOperation(Address addr, std::list<RTL *> *lrtl, OPER op);

	bool isHelperFunc(Address dest, Address addr, std::list<RTL *> *lrtl) override;
	void gen32op32gives64(OPER op, std::list<RTL *> *lrtl, Address addr);
	bool helperFuncLong(Address dest, Address addr, std::list<RTL *> *lrtl, QString& name);

	// void    setReturnLocations(CalleeEpilogue* epilogue, int iReg);

	// This struct represents a single nop instruction. Used as a substitute delay slot instruction
	DecodeResult nop_inst;
	IBinarySymbolTable *SymbolTable;
};
