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


/**
 * Contains routines to manage the decoding of st20
 * instructions and the instantiation to RTLs, removing st20
 * dependent features such as delay slots in the process. These
 * functions replace Frontend.cc for decoding sparc instructions.
 */
class ST20FrontEnd : public DefaultFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    ST20FrontEnd(BinaryFile *binaryFile, Prog *prog);
    ST20FrontEnd(const ST20FrontEnd& other) = delete;
    ST20FrontEnd(ST20FrontEnd&& other) = default;

    /// \copydoc IFrontEnd::~IFrontEnd
    virtual ~ST20FrontEnd() override = default;

    ST20FrontEnd& operator=(const ST20FrontEnd& other) = delete;
    ST20FrontEnd& operator=(ST20FrontEnd&& other) = default;

public:
    /// \copydoc IFrontEnd::processProc
    virtual bool processProc(Address entryAddr, UserProc *proc, QTextStream& os, bool frag = false, bool spec = false) override;

    /// \copydoc IFrontEnd::getDefaultParams
    virtual std::vector<SharedExp>& getDefaultParams() override;

    /// \copydoc IFrontEnd::getDefaultReturns
    virtual std::vector<SharedExp>& getDefaultReturns() override;

    /// \copydoc IFrontEnd::getMainEntryPoint
    virtual Address getMainEntryPoint(bool& gotMain) override;
};
