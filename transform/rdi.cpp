/*
 * Copyright (C) 2004, Mike Van Emmerik and Trent Waddington
 */
/***************************************************************************/ /**
  * \file    generic.cpp
  * \brief   Implementation of the RDIExpTransformer and related classes.
  *============================================================================*/

#include "rdi.h"

#include "types.h"
#include "statement.h"
#include "cfg.h"
#include "exp.h"
#include "register.h"
#include "rtl.h"
#include "proc.h"
#include "transformer.h"

#include <cassert>
#include <numeric>   // For accumulate
#include <algorithm> // For std::max()
#include <map>       // In decideType()
#include <sstream>   // Need gcc 3.0 or better

Exp *RDIExpTransformer::applyTo(Exp *e, bool &bMod) {
    if (e->getOper() == opAddrOf && e->getSubExp1()->getOper() == opMemOf) {
        e = e->getSubExp1()->getSubExp1()->clone();
        bMod = true;
    }
    return e;
}
