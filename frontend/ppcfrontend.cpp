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

/*==============================================================================
 * FILE:	   frontend/sparcfrontend.cpp
 * OVERVIEW:   This file contains routines to manage the decoding of sparc
 *			   instructions and the instantiation to RTLs, removing sparc
 *			   dependent features such as delay slots in the process. These
 *			   functions replace Frontend.cc for decoding sparc instructions.
 *============================================================================*/
/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#include <iomanip>			// For setfill etc
#include <sstream>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "exp.h"
#include "register.h"
#include "rtl.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "decoder.h"
#include "ppcdecoder.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "ppcfrontend.h"
#include "BinaryFile.h"		// E.g. IsDynamicallyLinkedProc
#include "boomerang.h"
#include "signature.h"

PPCFrontEnd::PPCFrontEnd(BinaryFile *pBF)
  : FrontEnd(pBF)
{
	decoder = new PPCDecoder();
}


// destructor
PPCFrontEnd::~PPCFrontEnd()
{
}


std::vector<Exp*> &PPCFrontEnd::getDefaultParams()
{
	static std::vector<Exp*> params;
	if (params.size() == 0) {
		for (int r=31; r>=0; r--) {
		  params.push_back(Location::regOf(r));
		}
	}
	return params;
}

std::vector<Exp*> &PPCFrontEnd::getDefaultReturns()
{
	static std::vector<Exp*> returns;
	if (returns.size() == 0) {
		for (int r=31; r>=0; r--) {
		  returns.push_back(Location::regOf(r));
		}

	}
	return returns;
}

ADDRESS PPCFrontEnd::getMainEntryPoint( bool &gotMain ) 
{
	gotMain = true;
	ADDRESS start = pBF->GetMainEntryPoint();
	if( start != NO_ADDRESS ) return start;

	start = pBF->GetEntryPoint();
	gotMain = false;
	if( start == NO_ADDRESS ) return NO_ADDRESS;

	gotMain = true;
	return start;
}


bool PPCFrontEnd::processProc(ADDRESS uAddr, UserProc* pProc, std::ofstream &os, bool frag /* = false */,
		bool spec /* = false */) {

	// Call the base class to do most of the work
	if (!FrontEnd::processProc(uAddr, pProc, os, frag, spec))
		return false;
	// This will get done twice; no harm
	pProc->setEntryBB();

	return true;
}
