#pragma once

// Class PPCFrontEnd: derived from FrontEnd, with source machine specific
// behaviour

#include <set>


#include "boomerang/frontend/Frontend.h"

class IFrontEnd;
class PPCDecoder;
class CallStatement;
struct DecodeResult;


class PPCFrontEnd : public IFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    PPCFrontEnd(IFileLoader *pLoader, Prog *Program);

    /// \copydoc IFrontEnd::~IFrontEnd
    virtual ~PPCFrontEnd();

    /// \copydoc IFrontEnd::getFrontEndId
    virtual Platform getType() const override { return Platform::PPC; }

    /// \copydoc IFrontEnd::processProc
    virtual bool processProc(Address uAddr, UserProc *pProc, QTextStream& os, bool frag = false, bool spec = false) override;

    /// \copydoc IFrontEnd::getDefaultParams
    virtual std::vector<SharedExp>& getDefaultParams() override;

    /// \copydoc IFrontEnd::getDefaultReturns
    virtual std::vector<SharedExp>& getDefaultReturns() override;

    /// \copydoc IFrontEnd::getMainEntryPoint
    virtual Address getMainEntryPoint(bool& gotMain) override;
};
