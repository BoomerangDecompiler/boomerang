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

class IFrontEnd;
class MIPSDecoder;
class CallStatement;
struct DecodeResult;


class MIPSFrontEnd : public IFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    MIPSFrontEnd(IFileLoader *pLoader, Prog *prog);
    MIPSFrontEnd(const MIPSFrontEnd& other) = delete;
    MIPSFrontEnd(MIPSFrontEnd&& other) = default;

    /// \copydoc IFrontEnd::~IFrontEnd
    virtual ~MIPSFrontEnd() override = default;

    MIPSFrontEnd& operator=(const MIPSFrontEnd& other) = delete;
    MIPSFrontEnd& operator=(MIPSFrontEnd&& other) = default;

public:
    /// \copydoc IFrontEnd::getFrontEndId
    virtual Platform getType() const override { return Platform::MIPS; }

    /// \copydoc IFrontEnd::processProc
    virtual bool processProc(Address uAddr, UserProc *pProc, QTextStream& os, bool frag = false, bool spec = false) override;

    /// \copydoc IFrontEnd::getDefaultParams
    virtual std::vector<SharedExp>& getDefaultParams() override;

    /// \copydoc IFrontEnd::getDefaultReturns
    virtual std::vector<SharedExp>& getDefaultReturns() override;

    /// \copydoc IFrontEnd::getMainEntryPoint
    virtual Address getMainEntryPoint(bool& gotMain) override;
};
