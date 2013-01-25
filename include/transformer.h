/*
 * Copyright (C) 2004, Trent Waddington
 */
/*==============================================================================
 * FILE:       transformer.h
 * OVERVIEW:   Provides the definition for the tranformer and related classes.
 *============================================================================*/
/*
 * $Revision$
 *
 * 17 Apr 04 - Trent: Created
 */

#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include <list>

class ExpTransformer
{
protected:
static std::list<ExpTransformer*> transformers;
public:
                    ExpTransformer();
virtual                ~ExpTransformer() { };        // Prevent gcc4 warning

static void            loadAll();

virtual Exp            *applyTo(Exp *e, bool &bMod) = 0;
static Exp            *applyAllTo(Exp *e, bool &bMod);
};

#endif

