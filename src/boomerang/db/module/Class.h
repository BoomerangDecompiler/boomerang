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


#include "boomerang/db/module/Module.h"
#include "boomerang/ssl/type/CompoundType.h"


class Class : public Module
{
protected:
    std::shared_ptr<CompoundType> m_type;

public:
    Class(const QString &name, Prog *_prog);

    /// A Class tends to be aggregated into the parent Module,
    /// this isn't the case with Java, but hey, we're not doing that yet.
    bool isAggregate() const override { return true; }
};
