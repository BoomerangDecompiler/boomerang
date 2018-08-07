#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ppcfrontend.h"


#include "boomerang/db/CFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/frontend/ppc/ppcdecoder.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/Register.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/util/log/Log.h"

#include <cassert>
#include <iomanip>
#include <sstream>


PPCFrontEnd::PPCFrontEnd(BinaryFile *binaryFile, Prog *prog)
    : DefaultFrontEnd(binaryFile, prog)
{
    m_decoder.reset(new PPCDecoder(prog));
}


std::vector<SharedExp>& PPCFrontEnd::getDefaultParams()
{
    static std::vector<SharedExp> params;

    if (params.size() == 0) {
        for (int r = 31; r >= 0; r--) {
            params.push_back(Location::regOf(r));
        }
    }

    return params;
}


std::vector<SharedExp>& PPCFrontEnd::getDefaultReturns()
{
    static std::vector<SharedExp> returns;

    if (returns.size() == 0) {
        for (int r = 31; r >= 0; r--) {
            returns.push_back(Location::regOf(r));
        }
    }

    return returns;
}


Address PPCFrontEnd::getMainEntryPoint(bool& gotMain)
{
    gotMain = true;
    Address start = m_binaryFile->getMainEntryPoint();

    if (start != Address::INVALID) {
        return start;
    }

    start   = m_binaryFile->getEntryPoint();
    gotMain = false;

    if (start == Address::INVALID) {
        return Address::INVALID;
    }

    gotMain = true;
    return start;
}


bool PPCFrontEnd::processProc(Address entryAddr, UserProc *proc, QTextStream& os, bool frag /* = false */,
                              bool spec /* = false */)
{
    // Call the base class to do most of the work
    if (!DefaultFrontEnd::processProc(entryAddr, proc, os, frag, spec)) {
        return false;
    }

    // This will get done twice; no harm
    proc->setEntryBB();

    return true;
}
