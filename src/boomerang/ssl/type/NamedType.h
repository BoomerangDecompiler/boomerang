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


class BOOMERANG_API NamedType : public Type
{
public:
    NamedType(const QString &_name);
    NamedType(const NamedType &other) = default;
    NamedType(NamedType &&other)      = default;

    virtual ~NamedType() override;

    NamedType &operator=(const NamedType &other) = default;
    NamedType &operator=(NamedType &&other) = default;

public:
    virtual bool isNamed() const override { return true; }
    QString getName() const { return name; }

    SharedType resolvesTo() const;

    static std::shared_ptr<NamedType> get(const QString &_name)
    {
        return std::make_shared<NamedType>(_name);
    }

    virtual SharedType clone() const override;

    virtual bool operator==(const Type &other) const override;
    virtual bool operator<(const Type &other) const override;


    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;

    /// \copydoc Type::meetWith
    virtual SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;
    virtual bool isCompatible(const Type &other, bool all) const override;

private:
    QString name;
};


template<>
inline std::shared_ptr<NamedType> Type::as<NamedType>()
{
    SharedType ty = shared_from_this();
    auto res      = std::dynamic_pointer_cast<NamedType>(ty);

    assert(res);
    return res;
}


template<>
inline std::shared_ptr<const NamedType> Type::as<NamedType>() const
{
    auto ty  = shared_from_this();
    auto res = std::dynamic_pointer_cast<const NamedType>(ty);

    assert(res);
    return res;
}
