#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/db/DefCollector.h"
#include "boomerang/ssl/exp/ExpHelp.h"
#include "boomerang/util/StatementList.h"


class CallStatement;
class Signature;


/**
 * A helper class for CallStatement::updateArguments. It just dishes out a new argument
 * from one of the three sources:
 *  - the signature,
 *  - the callee parameters,
 *  - or the defCollector in the call
 */
class ArgSourceProvider
{
public:
    enum class ArgSource
    {
        INVALID   = 0xFF,
        Lib       = 0,
        Callee    = 1,
        Collector = 2
    };

public:
    explicit ArgSourceProvider(CallStatement *call);

public:
    /// Get the next location (not subscripted)
    SharedExp nextArgLoc();

    /// Get the current location's type
    SharedType curType(SharedExp e);

    /// True if the given location (not subscripted) exists as a source
    bool exists(SharedExp loc);

    /// Localise to this call if necessary
    SharedExp localise(SharedExp e);

public:
    ArgSource src       = ArgSource::INVALID;
    CallStatement *call = nullptr;

    // For ArgSource::Lib
    int i = 0;
    int n = 0;
    std::shared_ptr<Signature> callSig;

    // For ArgSource::Callee
    StatementList::iterator pp;
    StatementList *calleeParams = nullptr;

    // For ArgSource::Collector
    DefCollector::iterator cc;
    DefCollector *defCol = nullptr;
};
