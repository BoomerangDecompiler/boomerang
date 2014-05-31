/*
 * Copyright (C) 2004, Mike Van Emmerik and Trent Waddington
 */
/***************************************************************************/ /**
  * \file        transformer.cpp
  * OVERVIEW:    Implementation of the Transformer and related classes.
  *============================================================================*/

#include "transformer.h"

#include "types.h"
#include "statement.h"
#include "cfg.h"
#include "exp.h"
#include "register.h"
#include "rtl.h"
#include "proc.h"
#include "boomerang.h"
#include "rdi.h"
#include "log.h"
#include "transformation-parser.h"

#include <cassert>
#include <numeric>   // For accumulate
#include <algorithm> // For std::max()
#include <map>       // In decideType()
#include <sstream>   // Need gcc 3.0 or better

std::list<ExpTransformer *> ExpTransformer::transformers;

ExpTransformer::ExpTransformer() { transformers.push_back(this); }

std::list<Exp *> cache;

Exp *ExpTransformer::applyAllTo(Exp *p, bool &bMod) {
    for (auto &elem : cache)
        if (*(elem)->getSubExp1() == *p)
            return (elem)->getSubExp2()->clone();

    Exp *e = p->clone();
    Exp *subs[3];
    subs[0] = e->getSubExp1();
    subs[1] = e->getSubExp2();
    subs[2] = e->getSubExp3();

    for (int i = 0; i < 3; i++)
        if (subs[i]) {
            bool mod = false;
            subs[i] = applyAllTo(subs[i], mod);
            if (mod && i == 0)
                e->setSubExp1(subs[i]);
            if (mod && i == 1)
                e->setSubExp2(subs[i]);
            if (mod && i == 2)
                e->setSubExp3(subs[i]);
            bMod |= mod;
            //            if (mod) i--;
        }

#if 0
    LOG << "applyAllTo called on " << e << "\n";
#endif
    bool mod;
    // do {
    mod = false;
    for (auto &transformer : transformers) {
        e = (transformer)->applyTo(e, mod);
        bMod |= mod;
    }
    //} while (mod);

    cache.push_back(Binary::get(opEquals, p->clone(), e->clone()));
    return e;
}

void ExpTransformer::loadAll() {
    std::string sPath = Boomerang::get()->getProgPath() + "transformations/exp.ts";

    std::ifstream ifs;
    ifs.open(sPath.c_str());

    if (!ifs.good()) {
        std::cerr << "can't open `" << sPath.c_str() << "'\n";
        exit(1);
    }

    while (!ifs.eof()) {
        std::string sFile;
        ifs >> sFile;
        size_t j = sFile.find('#');
        if (j != (size_t)-1)
            sFile = sFile.substr(0, j);
        if (sFile.size() > 0 && sFile[sFile.size() - 1] == '\n')
            sFile = sFile.substr(0, sFile.size() - 1);
        if (sFile == "")
            continue;
        std::ifstream ifs1;
        std::string sPath1 = Boomerang::get()->getProgPath() + "transformations/" + sFile;
        ifs1.open(sPath1.c_str());
        if (!ifs1.good()) {
            LOG << "can't open `" << sPath1.c_str() << "'\n";
            exit(1);
        }
        TransformationParser *p = new TransformationParser(ifs1, false);
        p->yyparse();
        ifs1.close();
    }
    ifs.close();
}
