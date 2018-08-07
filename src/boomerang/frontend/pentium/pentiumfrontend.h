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


#include "boomerang/frontend/DefaultFrontEnd.h"

#include <unordered_set>


/**
 * Class PentiumFrontEnd: derived from FrontEnd, with source machine specific
 * behaviour
 */
class PentiumFrontEnd : public DefaultFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    PentiumFrontEnd(BinaryFile *binaryFile, Prog *prog);
    PentiumFrontEnd(const PentiumFrontEnd& other) = delete;
    PentiumFrontEnd(PentiumFrontEnd&& other) = default;

    /// \copydoc IFrontEnd::~IFrontEnd
    virtual ~PentiumFrontEnd() override;

    PentiumFrontEnd& operator=(const PentiumFrontEnd& other) = delete;
    PentiumFrontEnd& operator=(PentiumFrontEnd&& other) = default;

public:
    /// \copydoc IFrontEnd::processProc
    virtual bool processProc(UserProc *proc, Address addr) override;

    /// \copydoc IFrontEnd::getMainEntryPoint
    /**
     * Locate the starting address of "main" in the code section.
     * \returns  Native pointer if found; Address::INVALID if not
     */
    virtual Address findMainEntryPoint(bool& gotMain) override;

protected:
    virtual bool decodeSingleInstruction(Address pc, DecodeResult& result) override;

    /// EXPERIMENTAL: can we find function pointers in arguments to calls this early?
    virtual void extraProcessCall(CallStatement *call, const RTLList& BB_rtls) override;

private:
    /**
     * Little simpler, just replaces FPUSH and FPOP with more complex
     * semantics.
     */
    void processFloatCode(Cfg *cfg);

    /**
     * Process a basic block, and all its successors, for floating point code.
     * Remove FPUSH/FPOP, instead decrementing or incrementing respectively the tos value to be used from
     * here down.
     *
     * \note tos has to be a parameter, not a global, to get the right value at any point in
     * the call tree
     *
     * \param bb pointer to the current BB
     * \param tos reference to the value of the "top of stack" pointer currently. Starts at zero, and is
     *        decremented to 7 with the first load, so r[39] should be used first, then r[38] etc. However, it is
     *        reset to 0 for calls, so that if a function returns a float, then it will always appear in r[32]
     * \param cfg passed to processFloatCode
     */
    void processFloatCode(BasicBlock *bb, int& tos, Cfg *cfg);

    /**
     * Process away %rpt and %skip in string instructions
     */
    void processStringInst(UserProc *proc);

    /**
     * Process for overlapped registers
     */
    void processOverlapped(UserProc *proc);

    /**
     * Checks for pentium specific helper functions like __xtol which have specific sematics.
     *
     * \note This needs to be handled in a resourcable way.
     *
     * \param dest the destination of this call
     * \param addr the address of this call instruction
     * \param lrtl a list of RTL pointers for this BB
     *
     * \returns true if a helper function is converted; false otherwise
     */
    bool isHelperFunc(Address dest, Address addr, RTLList& lrtl) override;

    /**
     * Finds a subexpression within this expression of the form
     * \code
     *   r[ int x] where min <= x <= max,
     * \endcode
     * and replaces it with
     * \code
     *   r[ int y] where y = min + (x - min + delta & mask).
     * \endcode
     * This is used to "flatten" stack floating point arithmetic (e.g. Pentium floating point code).
     * If registers are not replaced "all at once" like this, there can be subtle errors from
     * re-replacing already replaced registers
     *
     * \param exp   Expression to modify
     * \param min   minimum register numbers before any change is considered
     * \param max   maximum register numbers before any change is considered
     * \param delta amount to bump up the register number by
     * \param mask  see above
     */
    void bumpRegisterAll(SharedExp exp, int min, int max, int delta, int mask);

    bool decodeSpecial(Address pc, DecodeResult& r);
    bool decodeSpecial_out(Address pc, DecodeResult& r);
    bool decodeSpecial_invalid(Address pc, DecodeResult& r);

    bool isOverlappedRegsProcessed(const BasicBlock *bb) const
    {
        return m_overlappedRegsProcessed.find(bb) != m_overlappedRegsProcessed.end();
    }

    bool isFloatProcessed(const BasicBlock *bb) const
    {
        return m_floatProcessed.find(bb) != m_floatProcessed.end();
    }

private:
    std::unordered_set<const BasicBlock *> m_overlappedRegsProcessed;
    std::unordered_set<const BasicBlock *> m_floatProcessed;
};
