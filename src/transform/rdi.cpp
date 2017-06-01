/*
 * Copyright (C) 2004, Mike Van Emmerik and Trent Waddington
 */

/***************************************************************************/ /**
 * \file    generic.cpp
 * \brief   Implementation of the RDIExpTransformer and related classes.
 ******************************************************************************/

#include "rdi.h"

#include "include/types.h"
#include "include/statement.h"
#include "db/cfg.h"
#include "db/exp.h"
#include "include/register.h"
#include "include/rtl.h"
#include "db/proc.h"
#include "include/transformer.h"

#include <cassert>
#include <numeric>   // For accumulate
#include <algorithm> // For std::max()
#include <map>       // In decideType()
#include <sstream>   // Need gcc 3.0 or better

SharedExp RDIExpTransformer::applyTo(SharedExp e, bool& bMod)
{
	if ((e->getOper() == opAddrOf) && (e->getSubExp1()->getOper() == opMemOf)) {
		e    = e->getSubExp1()->getSubExp1()->clone();
		bMod = true;
	}

	return e;
}
