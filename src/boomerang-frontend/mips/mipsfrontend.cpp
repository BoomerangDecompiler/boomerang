/****************************************************************
 *
 * FILENAME
 *
 *   \file mipsfrontend.cpp
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

#include "mipsfrontend.h"

#include "boomerang/core/BinaryFileFactory.h" // E.g. IsDynamicallyLinkedProc
#include "boomerang/util/Log.h"

#include "boomerang/db/Register.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/exp/Location.h"

#include "boomerang-frontend/mips/mipsdecoder.h"


#include <cassert>
#include <iomanip> // For setfill etc
#include <sstream>


MIPSFrontEnd::MIPSFrontEnd(IFileLoader *pBF, Prog *prog, BinaryFileFactory *_pbff)
    : IFrontEnd(pBF, prog, _pbff)
{
    m_decoder = new MIPSDecoder(prog);
}


MIPSFrontEnd::~MIPSFrontEnd()
{
}


std::vector<SharedExp>& MIPSFrontEnd::getDefaultParams()
{
    static std::vector<SharedExp> params;

    if (params.size() == 0) {
        for (int r = 31; r >= 0; r--) {
            params.push_back(Location::regOf(r));
        }
    }

    return params;
}


std::vector<SharedExp>& MIPSFrontEnd::getDefaultReturns()
{
    static std::vector<SharedExp> returns;

    if (returns.size() == 0) {
        for (int r = 31; r >= 0; r--) {
            returns.push_back(Location::regOf(r));
        }
    }

    return returns;
}


Address MIPSFrontEnd::getMainEntryPoint(bool& gotMain)
{
    gotMain = true;
       Address start = m_fileLoader->getMainEntryPoint();

    if (start != Address::INVALID) {
        return start;
    }

    start   = m_fileLoader->getEntryPoint();
    gotMain = false;

    if (start == Address::INVALID) {
        return Address::INVALID;
    }

    gotMain = true;
    return start;
}


bool MIPSFrontEnd::processProc(Address uAddr, UserProc *pProc, QTextStream& os, bool frag /* = false */,
                               bool spec /* = false */)
{
    // Call the base class to do most of the work
    if (!IFrontEnd::processProc(uAddr, pProc, os, frag, spec)) {
        return false;
    }

    // This will get done twice; no harm
    pProc->setEntryBB();

    return true;
}
