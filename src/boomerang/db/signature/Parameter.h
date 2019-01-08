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


#include "boomerang/ssl/type/Type.h"


/**
 * A parameter of a function.
 */
class BOOMERANG_API Parameter
{
public:
    Parameter(SharedType type, const QString &name, SharedExp exp = nullptr,
              const QString &boundMax = "");

public:
    bool operator==(const Parameter &other) const;
    bool operator!=(const Parameter &other) const { return !(*this == other); }
    bool operator<(const Parameter &other) const;

public:
    /// Make a deep copy clone of this Parameter
    std::shared_ptr<Parameter> clone() const;

    /// \returns the type of this function parameter
    SharedType getType() const { return m_type; }

    /// \returns the name of this function paramter
    const QString &getName() const { return m_name; }

    SharedExp getExp() const { return m_exp; }
    QString getBoundMax() const { return m_boundMax; }

    void setType(SharedType ty) { m_type = ty; }
    void setName(const QString &name) { m_name = name; }
    void setExp(SharedExp e) { m_exp = e; }

    /// this parameter is the bound of another parameter with name \p name
    void setBoundMax(const QString &name);

private:
    SharedType m_type;
    QString m_name  = "";
    SharedExp m_exp = nullptr;
    QString m_boundMax;
};
