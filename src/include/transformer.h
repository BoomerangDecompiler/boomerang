/*
 * Copyright (C) 2004, Trent Waddington
 */
/***************************************************************************/ /**
  * \file       transformer.h
  * \brief   Provides the definition for the tranformer and related classes.
  ******************************************************************************/

#pragma once
#include <list>
#include <memory>
class Exp;
using SharedExp = std::shared_ptr<Exp>;
class ExpTransformer {
  protected:
    static std::list<ExpTransformer *> transformers;

  public:
    ExpTransformer();
    virtual ~ExpTransformer() {} // Prevent gcc4 warning

    static void loadAll();

    virtual SharedExp applyTo(SharedExp e, bool &bMod) = 0;
    static SharedExp applyAllTo(const SharedExp &e, bool &bMod);
};
