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
 * \file       ppcfrontend.cpp
 * \brief   This file contains routines to manage the decoding of ppc
 *               instructions and the instantiation to RTLs, removing sparc
 *               dependent features such as delay slots in the process. These
 *               functions replace Frontend.cc for decoding sparc instructions.
 ******************************************************************************/

/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/

#include "ppcfrontend.h"

#include "include/exp.h"
#include "include/register.h"
#include "include/rtl.h"
#include "db/cfg.h"
#include "include/proc.h"
#include "include/prog.h"
#include "include/decoder.h"
#include "ppcdecoder.h"
#include "boom_base/BinaryFile.h"
#include "include/frontend.h"
#include "boom_base/BinaryFile.h" // E.g. IsDynamicallyLinkedProc
#include "boom_base/log.h"
#include "include/signature.h"

#include <cassert>
#include <iomanip> // For setfill etc
#include <sstream>
PPCFrontEnd::PPCFrontEnd(QObject *pBF, Prog *prog, BinaryFileFactory *_pbff)
	: FrontEnd(pBF, prog, _pbff)
{
	decoder = new PPCDecoder(prog);
}


// destructor
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


ADDRESS PPCFrontEnd::getMainEntryPoint(bool& gotMain)
{
	gotMain = true;
	ADDRESS start = ldrIface->getMainEntryPoint();

	if (start != NO_ADDRESS) {
		return start;
	}

	start   = ldrIface->getEntryPoint();
	gotMain = false;

	if (start == NO_ADDRESS) {
		return NO_ADDRESS;
	}

	gotMain = true;
	return start;
}


bool PPCFrontEnd::processProc(ADDRESS uAddr, UserProc *pProc, QTextStream& os, bool frag /* = false */,
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
