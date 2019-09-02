#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/visitor/stmtmodifier/StmtModifier.h"


class BOOMERANG_API StmtSubscriptReplacer : public StmtModifier
{
public:
    StmtSubscriptReplacer(const SharedConstStmt &original, const SharedStmt &replacement);
    virtual ~StmtSubscriptReplacer() override = default;

public:
    /// \copydoc StmtModifier::visit
    virtual void visit(const std::shared_ptr<PhiAssign> &stmt, bool &visitChildren) override;
};
