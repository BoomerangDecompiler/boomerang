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


#include "boomerang/visitor/stmtmodifier/StmtModifier.h"


class ProcCFG;
class ImplicitConverter;


/**
 *
 */
class StmtImplicitConverter : public StmtModifier
{
public:
    StmtImplicitConverter(ImplicitConverter *ic, ProcCFG *cfg);
    virtual ~StmtImplicitConverter() = default;

public:
    /// \copydoc StmtModifier::visit
    void visit(const std::shared_ptr<PhiAssign> &stmt, bool &visitChildren) override;

private:
    ProcCFG *m_cfg;
};
