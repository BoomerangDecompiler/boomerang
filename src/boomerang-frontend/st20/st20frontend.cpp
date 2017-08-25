/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       st20frontend.cpp
 * \brief   This file contains routines to manage the decoding of st20
 *               instructions and the instantiation to RTLs, removing st20
 *               dependent features such as delay slots in the process. These
 *               functions replace Frontend.cc for decoding sparc instructions.
 ******************************************************************************/

/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/

#include "st20frontend.h"

#include "boomerang/util/Log.h"

#include "boomerang/db/Register.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/exp/Location.h"

#include "boomerang/loader/IFileLoader.h"
#include "boomerang-frontend/st20/st20decoder.h"

#include <cassert>
#include <iomanip> // For setfill etc
#include <sstream>


ST20FrontEnd::ST20FrontEnd(IFileLoader *pBF, Prog *prog)
    : IFrontEnd(pBF, prog)
{
    m_decoder = new ST20Decoder(prog);
}


ST20FrontEnd::~ST20FrontEnd()
{
}


std::vector<SharedExp>& ST20FrontEnd::getDefaultParams()
{
    static std::vector<SharedExp> params;

    if (params.size() == 0) {
        params.push_back(Location::memOf(Location::regOf(3)));
    }

    return params;
}


std::vector<SharedExp>& ST20FrontEnd::getDefaultReturns()
{
    static std::vector<SharedExp> returns;

    if (returns.size() == 0) {
        returns.push_back(Location::regOf(0));
        returns.push_back(Location::regOf(3));
        //        returns.push_back(Terminal::get(opPC));
    }

    return returns;
}


Address ST20FrontEnd::getMainEntryPoint(bool& gotMain)
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


bool ST20FrontEnd::processProc(Address uAddr, UserProc *pProc, QTextStream& os, bool frag /* = false */,
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
