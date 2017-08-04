/*
 * Copyright (C) 2004, Mike Van Emmerik and Trent Waddington
 */

/***************************************************************************/ /**
 * \file    generic.cpp
 * \brief   Implementation of the RDIExpTransformer and related classes.
 ******************************************************************************/

#include "RDIExpTransformer.h"

#include "boomerang/util/Types.h"

#include "boomerang/db/CFG.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/proc/Proc.h"
#include "boomerang/transform/ExpTransformer.h"
#include "boomerang/db/exp/Exp.h"

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
