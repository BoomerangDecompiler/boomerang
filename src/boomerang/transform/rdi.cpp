/*
 * Copyright (C) 2004, Mike Van Emmerik and Trent Waddington
 */

/***************************************************************************/ /**
 * \file    generic.cpp
 * \brief   Implementation of the RDIExpTransformer and related classes.
 ******************************************************************************/

#include "rdi.h"

#include "boomerang/util/types.h"

#include "boomerang/db/cfg.h"
#include "boomerang/db/register.h"
#include "boomerang/db/rtl.h"
#include "boomerang/db/proc.h"
#include "boomerang/include/transformer.h"

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
