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


class MIPSFrontEnd : public DefaultFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    MIPSFrontEnd(BinaryFile *binaryFile, Prog *prog);
    MIPSFrontEnd(const MIPSFrontEnd& other) = delete;
    MIPSFrontEnd(MIPSFrontEnd&& other) = default;

    /// \copydoc IFrontEnd::~IFrontEnd
    virtual ~MIPSFrontEnd() override = default;

    MIPSFrontEnd& operator=(const MIPSFrontEnd& other) = delete;
    MIPSFrontEnd& operator=(MIPSFrontEnd&& other) = default;

public:
    /// \copydoc IFrontEnd::processProc
    virtual bool processProc(UserProc *proc, Address entryAddr) override;

    /// \copydoc IFrontEnd::getMainEntryPoint
    virtual Address getMainEntryPoint(bool& gotMain) override;
};
