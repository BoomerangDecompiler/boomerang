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

#include "boomerang/db/exp.h"
#include "boomerang/db/register.h"
#include "boomerang/db/rtl.h"
#include "boomerang/db/cfg.h"
#include "boomerang/db/proc.h"
#include "boomerang/db/prog.h"
#include "boomerang/db/signature.h"

#include "boomerang/include/frontend.h"
#include "boomerang/include/decoder.h"

#include "boomerang-frontend/mips/mipsdecoder.h"


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
