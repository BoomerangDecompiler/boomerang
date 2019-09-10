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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/frontend/DefaultFrontEnd.h"

#include <unordered_set>


/**
 * Class PentiumFrontEnd: derived from FrontEnd, with source machine specific
 * behaviour
 */
class BOOMERANG_PLUGIN_API PentiumFrontEnd : public DefaultFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    PentiumFrontEnd(Project *project);
    PentiumFrontEnd(const PentiumFrontEnd &other) = delete;
    PentiumFrontEnd(PentiumFrontEnd &&other)      = default;

    /// \copydoc IFrontEnd::~IFrontEnd
    virtual ~PentiumFrontEnd() override;

    PentiumFrontEnd &operator=(const PentiumFrontEnd &other) = delete;
    PentiumFrontEnd &operator=(PentiumFrontEnd &&other) = default;

public:
    bool initialize(Project *project) override;

    /// \copydoc IFrontEnd::processProc
    virtual bool processProc(UserProc *proc, Address addr) override;

    /// \copydoc IFrontEnd::getMainEntryPoint
    virtual Address findMainEntryPoint(bool &gotMain) override;

    /// \copydoc IFrontEnd::decodeSingleInstruction
protected:
    /// \copydoc IFrontEnd::extraProcessCall
    /// EXPERIMENTAL: can we find function pointers in arguments to calls this early?
    virtual void extraProcessCall(CallStatement *call, const RTLList &BB_rtls) override;

private:
    /**
     * Process away %rpt and %skip in string instructions
     */
    void processStringInst(UserProc *proc);

    /**
     * Process for overlapped registers
     */
    void processOverlapped(UserProc *proc);

    /**
     * Checks for Pentium specific helper functions like __xtol which have specific sematics.
     *
     * \note This needs to be handled in a resourcable way.
     *
     * \param dest the destination of this call
     * \param addr the address of this call instruction
     * \param lrtl a list of RTL pointers for this BB
     *
     * \returns true if a helper function is converted; false otherwise
     */
    bool isHelperFunc(Address dest, Address addr, RTLList &lrtl) override;

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
