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


#include "boomerang/db/exp/Terminal.h"


/**
 * class TypeVal. Just a Terminal with a Type.
 * Used for type values in constraints
 */
class TypeVal : public Terminal
{
public:
    TypeVal(SharedType ty);
    virtual ~TypeVal() override;

    /// \copydoc Terminal::clone
    virtual SharedExp clone() const override;

    /// \copydoc Terminal::get
    static std::shared_ptr<TypeVal> get(const SharedType& st) { return std::make_shared<TypeVal>(st); }

    /// \copydoc Terminal::operator==
    bool operator==(const Exp& o) const override;

    /// \copydoc Terminal::operator<
    bool operator<(const Exp& o) const override;

    /// \copydoc Terminal::operator*=
    bool operator*=(const Exp& o) const override;

    /// \copydoc Terminal::print
    void print(QTextStream& os, bool = false) const override;

    /// \copydoc Terminal::printx
    void printx(int ind) const override;

    virtual SharedType getType() { return m_type; }
    virtual void setType(SharedType t) { m_type = t; }

    /// Should not be constraining constraints
    SharedExp genConstraints(SharedExp /*restrictTo*/) override
    {
        assert(false);
        return nullptr;
    }

    /// \copydoc Terminal::accept
    virtual bool accept(ExpVisitor *v) override;

    /// \copydoc Terminal::accept
    virtual SharedExp accept(ExpModifier *v) override;

private:
    SharedType m_type;
};
