/*
 * Copyright (C) 2004, Trent Waddington
 */
/*==============================================================================
 * FILE:       removedoubleindirection.h
 * OVERVIEW:   Provides the definition for the remove double indirection exp 
               tranformer.
 *============================================================================*/
/*
 * $Revision$
 *
 * 17 Apr 04 - Trent: Created
 */

#ifndef REMOVE_DOUBLE_INDIRECTION_H
#define REMOVE_DOUBLE_INDIRECTION_H

class RDIExpTransformer : public ExpTransformer
{
public:
    RDIExpTransformer() { }
    virtual Exp *applyTo(Exp *e, bool &bMod);
};

#endif

