#pragma once

/*
 * Copyright (C) 2004, Trent Waddington
 */

/***************************************************************************/ /**
 * \file       removedoubleindirection.h
 * OVERVIEW:   Provides the definition for the remove double indirection exp
 *              tranformer.
 ******************************************************************************/

#include "boomerang/include/transformer.h"

class RDIExpTransformer : public ExpTransformer
{
public:
	RDIExpTransformer() {}
	virtual SharedExp applyTo(SharedExp e, bool& bMod) override;
};
