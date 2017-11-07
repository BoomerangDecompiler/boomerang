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


#include "boomerang/db/visitor/StmtModifier.h"


class Cfg;
class ImplicitConverter;


/**
 *
 */
class StmtImplicitConverter : public StmtModifier
{
public:
    StmtImplicitConverter(ImplicitConverter *ic, Cfg *cfg);

    virtual void visit(PhiAssign *s, bool& recur) override;

private:
    Cfg *m_cfg;
};
