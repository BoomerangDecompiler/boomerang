/*
 * Copyright (C) 2004, Trent Waddington
 */
/***************************************************************************/ /**
  * \file       generic.h
  * \brief   Provides the definition for the generic exp tranformer.
  ******************************************************************************/

#ifndef GENERIC_EXP_TRANSFORMER_H
#define GENERIC_EXP_TRANSFORMER_H

#include "transformer.h"

class GenericExpTransformer : public ExpTransformer {
  protected:
    Exp *match, *where, *become;

    bool checkCond(Exp *cond, Exp *bindings);
    Exp *applyFuncs(Exp *rhs);

  public:
    GenericExpTransformer(Exp *_match, Exp *_where, Exp *_become) : match(_match), where(_where), become(_become) {}
    virtual Exp *applyTo(Exp *e, bool &bMod);
};

#endif
