/*
 * Copyright (C) 2004, Trent Waddington
 */
/***************************************************************************/ /**
  * \file       removedoubleindirection.h
  * OVERVIEW:   Provides the definition for the remove double indirection exp
                tranformer.
  *============================================================================*/

#ifndef REMOVE_DOUBLE_INDIRECTION_H
#define REMOVE_DOUBLE_INDIRECTION_H

#include "transformer.h"

class RDIExpTransformer : public ExpTransformer {
  public:
    RDIExpTransformer() {}
    virtual Exp *applyTo(Exp *e, bool &bMod);
};

#endif
