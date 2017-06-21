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

#include "db/exp.h"
#include "db/register.h"
#include "db/rtl.h"
#include "db/cfg.h"
#include "db/proc.h"
#include "db/prog.h"
#include "include/decoder.h"
#include "mipsdecoder.h"
#include "core/BinaryFileFactory.h"
#include "include/frontend.h"
#include "core/BinaryFileFactory.h" // E.g. IsDynamicallyLinkedProc
#include "core/log.h"
#include "db/signature.h"

#include <cassert>
#include <iomanip> // For setfill etc
#include <sstream>


MIPSFrontEnd::MIPSFrontEnd(IFileLoader *pBF, Prog *prog, BinaryFileFactory *_pbff)
	: FrontEnd(pBF, prog, _pbff)
{
	decoder = new MIPSDecoder(prog);
}


// destructor
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


ADDRESS MIPSFrontEnd::getMainEntryPoint(bool& gotMain)
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


bool MIPSFrontEnd::processProc(ADDRESS uAddr, UserProc *pProc, QTextStream& os, bool frag /* = false */,
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
