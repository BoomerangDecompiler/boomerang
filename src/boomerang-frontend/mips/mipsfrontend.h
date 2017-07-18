#pragma once

/****************************************************************
 *
 * FILENAME
 *
 *   \file mipsfrontend.h
 *
 * PURPOSE
 *
 *   Skeleton for MIPS disassembly.
 *
 * AUTHOR
 *
 *   \author Markus Gothe, nietzsche@lysator.liu.se
 *
 * REVISION
 *
 *   $Id$
 *
 *****************************************************************/

// Class MIPSFrontEnd: derived from FrontEnd, with source machine specific
// behaviour

#include <set>


#include "boomerang/frontend/frontend.h"

class IFrontEnd;
class MIPSDecoder;
class CallStatement;
struct DecodeResult;


class MIPSFrontEnd : public IFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    MIPSFrontEnd(IFileLoader *pLoader, Prog *prog, BinaryFileFactory *pbff);
    
    /// \copydoc IFrontEnd::~IFrontEnd
    virtual ~MIPSFrontEnd();

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
