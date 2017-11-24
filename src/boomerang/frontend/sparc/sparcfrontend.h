#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/frontend/Frontend.h"
#include "boomerang/frontend/Decoder.h"
#include "boomerang/type/type/Type.h"
#include "boomerang/db/exp/Operator.h"

class IFrontEnd;
class SparcDecoder;
class CallStatement;
class IBinarySymbolTable;

struct DecodeResult;


/**
 * This file contains routines to manage the decoding of sparc instructions and the instantiation to RTLs,
 * removing sparc dependent features such as delay slots in the process. These functions replace
 * frontend.cpp for decoding sparc instructions.
 */
class SparcFrontEnd : public IFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    SparcFrontEnd(IFileLoader *p_BF, Prog *prog);
    SparcFrontEnd(const SparcFrontEnd& other) = delete;
    SparcFrontEnd(SparcFrontEnd&& other) = default;

    virtual ~SparcFrontEnd() = default;

    SparcFrontEnd& operator=(const SparcFrontEnd&) = delete;
    SparcFrontEnd& operator=(SparcFrontEnd&&) = default;

public:
    /// \copydoc IFrontEnd::getType
    virtual Platform getType() const override { return Platform::SPARC; }

    /**
     * \copydoc IFrontEnd::processProc
     *
     * This overrides the base class processProc to do source machine
     * specific things (but often calls the base class to do most of the
     * work. Sparc is an exception)
     */
    /**
     * Builds the CFG for a procedure out of the RTLs constructed
     *         during decoding. The semantics of delayed CTIs are
     *         transformed into CTIs that aren't delayed.
     * \note   This function overrides (and replaces) the function with
     *         the same name in class FrontEnd. The required actions
     *         are so different that the base class implementation
     *         can't be re-used
     * \param uAddr - the native address at which the procedure starts
     * \param proc - the procedure object
     * \param os - output stream for rtl output
     * \param fragment - TODO: needs documentation.
     * \param spec - if true, this is a speculative decode
     * \returns              True if a good decode
     */
    virtual bool processProc(Address uAddr, UserProc *proc, QTextStream& os, bool fragment = false, bool spec = false) override;

    /// \copydoc IFrontEnd::getDefaultParams
    virtual std::vector<SharedExp>& getDefaultParams() override;

    /// \copydoc IFrontEnd::getDefaultReturns
    virtual std::vector<SharedExp>& getDefaultReturns() override;

    /// \copydoc IFrontEnd::getMainEntryPoint
    /**
     * Locate the starting address of "main" in the code section
     * \param gotMain set if main found
     * \returns Native pointer if found; Address::INVALID if not
     */
    virtual Address getMainEntryPoint(bool& gotMain) override;

private:
    /**
     * Emit a warning when encountering a DCTI couple.
     * \param uAt - the address of the couple
     * \param uDest - the address of the first DCTI in the couple
     */
    void warnDCTcouple(Address uAt, Address uDest);

    /**
     * Check if delay instruction can be optimized.
     *
     * Determines if a delay instruction is exactly the same as the instruction immediately preceding the
     * destination of a CTI; i.e. has been copied from the real destination to the delay slot as an
     * optimisation
     *
     * \param src - the logical source address of a CTI
     * \param dest - the logical destination address of the CTI
     * \param delta - used to convert logical to real addresses
     * \param uUpper - first address past the end of the main text section
     * SIDE EFFECT:        Optionally displays an error message if the target of the branch is the delay slot of another
     *                    delayed CTI
     * \returns true if delay instruction can be optimized away
     */
    bool optimise_DelayCopy(Address src, Address dest, ptrdiff_t delta, Address uUpper);

    /**
     * Determines if the given call and delay instruction consitute a call
     * where callee returns to the caller's caller. That is:
     *   ProcA      |  ProcB:       |         ProcC:
     * -------------|---------------|---------------
     * ...          | ...           |        ...
     * call ProcB   | call ProcC    |        ret
     * ...          | restore       |        ...
     *
     * The restore instruction in ProcB will effectively set %o7 to be %i7,
     * the address to which ProcB will return. So in effect ProcC will return
     * to ProcA at the position just after the call to ProcB. This is equivalent
     * to ProcC returning to ProcB which then immediately returns to ProcA.
     *
     * \note       We don't set a label at the return, because we also have to
     *             force a jump at the call BB, and in some cases we don't
     *             have the call BB as yet. So these two are up to the caller
     * \note       The name of this function is now somewhat irrelevant.
     *             The whole function is somewhat irrelevant; it dates from
     *             the times when you would always find an actual restore
     *             in the delay slot. With some patterns, this is no longer true.
     *
     * \param      call  the RTL for the caller (e.g. "call ProcC" above)
     * \param      rtl   pointer to the RTL for the call instruction
     * \param      delay the RTL for the delay instruction (e.g. "restore")
     * \returns    The basic block containing the single return instruction
     *             if this optimisation applies, nullptr otherwise.
     */
    BasicBlock *optimise_CallReturn(CallStatement *call, RTL *rtl, RTL *delay, UserProc *pProc);

    /**
     * Adds the destination of a branch to the queue of address
     * that must be decoded (if this destination has not already been visited).
     * but newBB may be changed if the destination of the branch is in the middle of an existing
     * BB. It will then be changed to point to a new BB beginning with the dest
     * \param newBB - the new basic block delimited by the branch instruction. May be nullptr if this block has been
     *        built before.
     * \param dest - the destination being branched to
     * \param hiAddress - the last address in the current procedure
     * \param cfg - the CFG of the current procedure
     * \param tq Object managing the target queue
     */
    void handleBranch(Address dest, Address hiAddress, BasicBlock *& newBB, Cfg *cfg, TargetQueue& tq);

    /**
     * Records the fact that there is a procedure at a given address. Also adds the out edge to the
     * lexical successor of the call site (taking into consideration the delay slot and possible UNIMP
     * instruction).
     *
     * \param  proc - caller - only used to access Prog
     * \param  dest - the address of the callee
     * \param  callBB - the basic block delimited by the call
     * \param  cfg - CFG of the enclosing procedure
     * \param  address - the address of the call instruction
     * \param  offset - the offset from the call instruction to which an outedge must be added. A value of 0
     *         means no edge is to be added.
     */
    void handleCall(UserProc *proc, Address dest, BasicBlock *callBB, Cfg *cfg, Address address, int offset = 0);

    /**
     * This is the stub for cases of DCTI couples that we haven't written
     * analysis code for yet. It simply displays an informative warning and returns.
     * \param         addr - the address of the first CTI in the couple
     *
     */
    void case_unhandled_stub(Address addr);

    /**
     * Handles a call instruction
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
     */
    bool case_CALL(Address& address, DecodeResult& inst, DecodeResult& delay_inst, std::list<RTL *> *& BB_rtls,
                   UserProc *proc, std::list<CallStatement *>& callList, QTextStream& os, bool isPattern = false);

    /**
     * Handles a non-call, static delayed (SD) instruction
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
     */
    void case_SD(Address& address, ptrdiff_t delta, Address hiAddress, DecodeResult& inst, DecodeResult& delay_inst,
                 std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq, QTextStream& os);

    /**
     * Handles all dynamic delayed jumps (jmpl, also dynamic calls)
     * \param address - the native address of the DD
     * \param delta - the offset of the above address from the logical address at which the procedure
     *                starts (i.e. the one given by dis)
     * \param inst - the info summaries when decoding the SD instruction
     * \param delay_inst - the info summaries when decoding the delay instruction
     * \param BB_rtls - the list of RTLs currently built for the BB under construction
     * \param tq Object managing the target queue
     * \param proc pointer to the current Proc object
     * \param callList - a set of pointers to CallStatements for procs yet to be processed
     * SIDE EFFECTS:     address may change; BB_rtls may be appended to or set nullptr
     * \returns              true if next instruction is to be fetched sequentially from this one
     */
    bool case_DD(Address& address, ptrdiff_t delta, DecodeResult& inst, DecodeResult& delay_inst,
                 std::list<RTL *> *& BB_rtls, TargetQueue& tq, UserProc *proc, std::list<CallStatement *>& callList);


    /**
     * Handles all Static Conditional Delayed non-anulled branches
     * \param   address - the native address of the DD
     * \param   delta - the offset of the above address from the logical address at which the procedure starts
     *                  (i.e. the one given by dis)
     * \param   hiAddress - first address outside this code section
     * \param   inst - the info summaries when decoding the SD instruction
     * \param   delay_inst - the info summaries when decoding the delay instruction
     * \param   BB_rtls - the list of RTLs currently built for the BB under construction
     * \param   cfg - the CFG of the enclosing procedure
     * \param   tq Object managing the target queue
     * SIDE EFFECTS:     address may change; BB_rtls may be appended to or set nullptr
     * \returns true if next instruction is to be fetched sequentially from this one
     */
    bool case_SCD(Address& address, ptrdiff_t delta, Address hiAddress, DecodeResult& inst, DecodeResult& delay_inst,
                  std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq);

    /**
     * Handles all static conditional delayed anulled branches followed by
     * an NCT (but not NOP) instruction.
     * \param address - the native address of the DD
     * \param delta - the offset of the above address from the logical
     *                address at which the procedure starts (i.e. the one given by dis)
     * \param hiAddress - first address outside this code section
     * \param  inst - the info summaries when decoding the SD instruction
     * \param delay_inst - the info summaries when decoding the delay instruction
     * \param BB_rtls - the list of RTLs currently built for the BB under construction
     * \param cfg - the CFG of the enclosing procedure
     * \param tq Object managing the target queue
     * SIDE EFFECTS:    address may change; BB_rtls may be appended to or set nullptr
     * \returns             true if next instruction is to be fetched sequentially from this one
     */
    bool case_SCDAN(Address& address, ptrdiff_t delta, Address hiAddress, DecodeResult& inst, DecodeResult& delay_inst,
                    std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq);

    /**
     * Emit a null RTL with the given address.
     * \param   pRtls - List of RTLs to append this instruction to
     * \param   uAddr - Native address of this instruction
     */
    void emitNop(std::list<RTL *> *pRtls, Address uAddr);

    /**
     * Emit the RTL for a call $+8 instruction, which is merely %o7 = %pc
     * \note   Assumes that the delay slot RTL has already been pushed; we must push the semantics BEFORE that RTL,
     *         since the delay slot instruction may use %o7. Example:
     *         CALL $+8            ! This code is common in startup code
     *         ADD     %o7, 20, %o0
     * \param pRtls - list of RTLs to append to
     * \param uAddr - native address for the RTL
     *
     */
    void emitCopyPC(std::list<RTL *> *pRtls, Address uAddr);
    unsigned fetch4(unsigned char *ptr);

    // Append one assignment to a list of RTLs
    void appendAssignment(const SharedExp& lhs, const SharedExp& rhs, SharedType type, Address addr, std::list<RTL *> *lrtl);

    /*
     * Small helper function to build an expression with
     * *128* m[m[r[14]+64]] = m[r[8]] OP m[r[9]]
     */
    void quadOperation(Address addr, std::list<RTL *> *lrtl, OPER op);

    /**
     * Checks for sparc specific helper functions like .urem, which have specific sematics.
     * Determine if this is a helper function, e.g. .mul. If so, append the appropriate RTLs to lrtl, and return true
     * \note   This needs to be handled in a resourcable way.
     * \param  dest destination of the call (native address)
     * \param  addr address of current instruction (native addr)
     * \param  lrtl list of RTL* for current BB
     * \returns True if a helper function was found and handled; false otherwise
     */
    bool isHelperFunc(Address dest, Address addr, std::list<RTL *> *lrtl) override;

    /**
     * Another small helper function to generate either (for V9):
     * 64* tmp[tmpl] = sgnex(32, 64, r8) op sgnex(32, 64, r9)
     * 32* r8 = truncs(64, 32, tmp[tmpl])
     * 32* r9 = r[tmpl]@32:63
     * or for v8:
     * 32* r[tmp] = r8 op r9
     * 32* r8 = r[tmp]
     * 32* r9 = %Y
     */
    void gen32op32gives64(OPER op, std::list<RTL *> *lrtl, Address addr);

    /// This is the long version of helperFunc (i.e. -f not used).
    /// This does the complete 64 bit semantics
    bool helperFuncLong(Address dest, Address addr, std::list<RTL *> *lrtl, QString& name);

    // This struct represents a single nop instruction.
    // Used as a substitute delay slot instruction
    DecodeResult nop_inst;
    IBinarySymbolTable *SymbolTable;
};
