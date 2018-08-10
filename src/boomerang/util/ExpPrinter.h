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


#include "boomerang/ssl/exp/ExpHelp.h"
#include "boomerang/ssl/exp/Operator.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"


class OStream;


class ExpPrinter
{
public:
    /// print \p exp to \p os
    void print(OStream& os, const Exp& exp, bool html = false) const;

private:
    /// print \p exp to \p os
    void print(OStream& os, const std::shared_ptr<const Exp>& exp) const;

    /// print \p exp to \p os
    void printHTML(OStream& os, const std::shared_ptr<const Exp>& exp) const;

    /**
     * Given an expression, and an immediate child expression, determine if
     * the child expression needs to be parenthesized or not.
     */
    bool childNeedsParentheses(const SharedConstExp& exp, const SharedConstExp& child) const;
};
