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

#include "core/BinaryFileFactory.h" // E.g. IsDynamicallyLinkedProc
#include "util/Log.h"

#include "db/exp.h"
#include "db/register.h"
#include "db/rtl.h"
#include "db/cfg.h"
#include "db/proc.h"
#include "db/prog.h"
#include "db/signature.h"

#include "frontend/st20/st20decoder.h"

#include "include/decoder.h"
#include "include/frontend.h"

#include <cassert>
#include <iomanip> // For setfill etc
#include <sstream>


ST20FrontEnd::ST20FrontEnd(IFileLoader *pBF, Prog *prog, BinaryFileFactory *_pbff)
	: FrontEnd(pBF, prog, _pbff)
{
	decoder = new ST20Decoder(prog);
}


// destructor
ST20FrontEnd::~ST20FrontEnd()
{
}


std::vector<SharedExp>& ST20FrontEnd::getDefaultParams()
{
	static std::vector<SharedExp> params;

	if (params.size() == 0) {
#if 0
		for (int r = 0; r <= 2; r++) {
			params.push_back(Location::regOf(r));
		}
#endif
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


ADDRESS ST20FrontEnd::getMainEntryPoint(bool& gotMain)
{
	gotMain = true;
	ADDRESS start = m_fileLoader->getMainEntryPoint();

	if (start != NO_ADDRESS) {
		return start;
	}

	start   = m_fileLoader->getEntryPoint();
	gotMain = false;

	if (start == NO_ADDRESS) {
		return NO_ADDRESS;
	}

	gotMain = true;
	return start;
}


bool ST20FrontEnd::processProc(ADDRESS uAddr, UserProc *pProc, QTextStream& os, bool frag /* = false */,
							   bool spec /* = false */)
{
	// Call the base class to do most of the work
	if (!FrontEnd::processProc(uAddr, pProc, os, frag, spec)) {
		return false;
	}

	// This will get done twice; no harm
	pProc->setEntryBB();

	return true;
}
