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
class ST20Decoder;
class CallStatement;

struct DecodeResult;


/**
 * Contains routines to manage the decoding of st20
 * instructions and the instantiation to RTLs, removing st20
 * dependent features such as delay slots in the process. These
 * functions replace Frontend.cc for decoding sparc instructions.
 */
class ST20FrontEnd : public IFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    ST20FrontEnd(IFileLoader *pLoader, Prog *prog);

    /// \copydoc IFrontEnd::~IFrontEnd
    virtual ~ST20FrontEnd();

    /// \copydoc IFrontEnd::getType
    virtual Platform getType() const override { return Platform::ST20; }

    /// \copydoc IFrontEnd::processProc
    virtual bool processProc(Address uAddr, UserProc *pProc, QTextStream& os, bool frag = false, bool spec = false) override;

    /// \copydoc IFrontEnd::getDefaultParams
    virtual std::vector<SharedExp>& getDefaultParams() override;

    /// \copydoc IFrontEnd::getDefaultReturns
    virtual std::vector<SharedExp>& getDefaultReturns() override;

    /// \copydoc IFrontEnd::getMainEntryPoint
    virtual Address getMainEntryPoint(bool& gotMain) override;
};
