#pragma once

// Class ST20FrontEnd: derived from FrontEnd, with source machine specific
// behaviour

#include <set>

#include "boomerang/db/exp.h"           // Ugh... just for enum OPER

#include "boomerang/frontend/frontend.h"

class IFrontEnd;
class ST20Decoder;
class CallStatement;

struct DecodeResult;


class ST20FrontEnd : public IFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    ST20FrontEnd(IFileLoader *pLoader, Prog *prog, BinaryFileFactory *pbff);

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
