#pragma once

/*
 * Copyright (C) 2004, Trent Waddington
 */

/***************************************************************************/ /**
 * \file       generic.h
 * \brief   Provides the definition for the generic exp tranformer.
 ******************************************************************************/

#include "include/transformer.h"

class GenericExpTransformer : public ExpTransformer
{
protected:
	SharedExp match, where, become;

	bool checkCond(SharedExp cond, SharedExp bindings);
	SharedExp applyFuncs(SharedExp rhs);

public:
	GenericExpTransformer(SharedExp _match, SharedExp _where, SharedExp _become)
		: match(_match)
		, where(_where)
		, become(_become) {}
	virtual SharedExp applyTo(SharedExp e, bool& bMod);
};
