#pragma once

/*
 * Copyright (C) 2004, Trent Waddington
 */

/***************************************************************************/ /**
 * \file       transformer.h
 * \brief   Provides the definition for the transformer and related classes.
 ******************************************************************************/

#include <list>
#include <memory>

class Exp;
using SharedExp = std::shared_ptr<Exp>;

class ExpTransformer
{
public:
    ExpTransformer();
    virtual ~ExpTransformer() {}

    static void loadAll();

    virtual SharedExp applyTo(SharedExp e, bool& bMod) = 0;
    static SharedExp applyAllTo(const SharedExp& e, bool& bMod);

protected:
    static std::list<ExpTransformer *> transformers;
};
