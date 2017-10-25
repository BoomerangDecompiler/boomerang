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


/**
 * \file       ppcfrontend.cpp
 * \brief   This file contains routines to manage the decoding of ppc
 *               instructions and the instantiation to RTLs, removing sparc
 *               dependent features such as delay slots in the process. These
 *               functions replace Frontend.cc for decoding sparc instructions.
 */

/**
 * Dependencies.
 */


#include "boomerang/db/Register.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/util/Log.h"

#include "boomerang/loader/IFileLoader.h"
#include "boomerang/frontend/ppc/ppcdecoder.h"

#include <cassert>
#include <iomanip> // For setfill etc
#include <sstream>


PPCFrontEnd::PPCFrontEnd(IFileLoader *pBF, Prog *prog)
    : IFrontEnd(pBF, prog)
{
    m_decoder = new PPCDecoder(prog);
}


PPCFrontEnd::~PPCFrontEnd()
{
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


bool PPCFrontEnd::processProc(Address uAddr, UserProc *pProc, QTextStream& os, bool frag /* = false */,
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
