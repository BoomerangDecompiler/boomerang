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


class IRFragment;


/**
 * Class X86FrontEnd: derived from FrontEnd, with source machine specific
 * behaviour
 */
class BOOMERANG_PLUGIN_API X86FrontEnd : public DefaultFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    X86FrontEnd(Project *project);
    X86FrontEnd(const X86FrontEnd &other) = delete;
    X86FrontEnd(X86FrontEnd &&other)      = default;

    /// \copydoc IFrontEnd::~IFrontEnd
    ~X86FrontEnd() override;

    X86FrontEnd &operator=(const X86FrontEnd &other) = delete;
    X86FrontEnd &operator=(X86FrontEnd &&other) = default;

public:
    bool initialize(Project *project) override;

    /// \copydoc IFrontEnd::processProc
    bool processProc(UserProc *proc, Address addr) override;

    /// \copydoc IFrontEnd::getMainEntryPoint
    Address findMainEntryPoint(bool &gotMain) override;

protected:
    /// \copydoc IFrontEnd::extraProcessCall
    /// EXPERIMENTAL: can we find function pointers in arguments to calls this early?
    virtual void extraProcessCall(IRFragment *callFrag) override;

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
     * Checks for x86 specific helper functions like __xtol which have specific sematics.
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

    bool isOverlappedRegsProcessed(const IRFragment *bb) const
    {
        return m_overlappedRegsProcessed.find(bb) != m_overlappedRegsProcessed.end();
    }

    bool isFloatProcessed(const IRFragment *bb) const
    {
        return m_floatProcessed.find(bb) != m_floatProcessed.end();
    }

private:
    std::unordered_set<const IRFragment *> m_overlappedRegsProcessed;
    std::unordered_set<const IRFragment *> m_floatProcessed;
};
