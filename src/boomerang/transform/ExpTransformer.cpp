#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpTransformer.h"


/**
 * \file        transformer.cpp
 * OVERVIEW:    Implementation of the Transformer and related classes.
 */
#include "boomerang/core/Boomerang.h"

#include "boomerang/db/CFG.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/proc/Proc.h"
#include "boomerang/db/exp/Binary.h"

#include "boomerang/util/Types.h"
#include "boomerang/transform/RDIExpTransformer.h"
#include "boomerang/transform/transformation-parser.h"

#include "boomerang/util/Log.h"
#include "boomerang/util/Types.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <cassert>
#include <numeric>   // For accumulate
#include <algorithm> // For std::max()
#include <map>       // In decideType()
#include <sstream>   // Need gcc 3.0 or better


std::list<ExpTransformer *> ExpTransformer::transformers;

ExpTransformer::ExpTransformer()
{
    transformers.push_back(this);
}


std::list<SharedExp> cache;

SharedExp ExpTransformer::applyAllTo(const SharedExp& p, bool& bMod)
{
    for (auto& elem : cache) {
        if (*(elem)->getSubExp1() == *p) {
            return (elem)->getSubExp2()->clone();
        }
    }

    SharedExp e = p->clone();
    SharedExp subs[3];
    subs[0] = e->getSubExp1();
    subs[1] = e->getSubExp2();
    subs[2] = e->getSubExp3();

    for (int i = 0; i < 3; i++) {
        if (subs[i]) {
            bool mod = false;
            subs[i] = applyAllTo(subs[i], mod);

            if (mod && (i == 0)) {
                e->setSubExp1(subs[i]);
            }

            if (mod && (i == 1)) {
                e->setSubExp2(subs[i]);
            }

            if (mod && (i == 2)) {
                e->setSubExp3(subs[i]);
            }

            bMod |= mod;
            //            if (mod) i--;
        }
    }

    bool mod;
    // do {
    mod = false;

    for (auto& transformer : transformers) {
        e     = (transformer)->applyTo(e, mod);
        bMod |= mod;
    }

    // } while (mod);

    cache.push_back(Binary::get(opEquals, p->clone(), e->clone()));
    return e;
}


void ExpTransformer::loadAll()
{
    QString sPath = Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("transformations/exp.ts");

    QFile file(sPath);

    if (!file.open(QFile::ReadOnly)) {
        LOG_ERROR("Can't open transformation file `%1'", sPath);
        return;
    }

    QTextStream ifs(&file);

    while (!ifs.atEnd()) {
        QString sFile;
        ifs >> sFile;
        sFile = sFile.mid(0, sFile.indexOf('#')).trimmed(); // remove comment and leading/trailing whitespaces

        if (sFile.isEmpty()) {
            continue;
        }

        std::ifstream ifs1;
        QString       sPath1 = Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("transformations/" + sFile);

        ifs1.open(sPath1.toStdString());

        if (!ifs1.good()) {
            LOG_ERROR("Can't open transformation file '%1'", sPath);
            return;
        }

        TransformationParser *p = new TransformationParser(ifs1, false);
        p->yyparse();
        ifs1.close();
    }
}
