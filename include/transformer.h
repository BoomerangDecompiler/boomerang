/*
 * Copyright (C) 2004, Trent Waddington
 */
/***************************************************************************/ /**
  * \file       transformer.h
  * \brief   Provides the definition for the tranformer and related classes.
  ******************************************************************************/

#pragma once
#include <list>
class Exp;
class ExpTransformer {
  protected:
    static std::list<ExpTransformer *> transformers;

  public:
    ExpTransformer();
    virtual ~ExpTransformer() {} // Prevent gcc4 warning

    static void loadAll();

    virtual Exp *applyTo(Exp *e, bool &bMod) = 0;
    static Exp *applyAllTo(Exp *e, bool &bMod);
};
