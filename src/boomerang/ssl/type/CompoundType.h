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

#include <vector>


/**
 * The compound type represents aggregate types like structures or classes.
 */
class BOOMERANG_API CompoundType : public Type
{
public:
    /// Constructs an empty compound type.
    explicit CompoundType();

    CompoundType(CompoundType &other)  = default;
    CompoundType(CompoundType &&other) = default;

    virtual ~CompoundType() override;

    CompoundType &operator=(CompoundType &other) = default;
    CompoundType &operator=(CompoundType &&other) = default;

public:
    /// \copydoc Type::operator==
    virtual bool operator==(const Type &other) const override;

    /// \copydoc Type::operator<
    virtual bool operator<(const Type &other) const override;

public:
    /// \copydoc Type::clone
    virtual SharedType clone() const override;

    static std::shared_ptr<CompoundType> get()
    {
        return std::make_shared<CompoundType>();
    }

public:
    /// \copydoc Type::getSize
    virtual Size getSize() const override;

    /// \copydoc Type::meetWith
    virtual SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

    /// \copydoc Type::getCtype
    virtual QString getCtype(bool final = false) const override;

    /// \copydoc Type::isCompatibleWith
    virtual bool isCompatibleWith(const Type &other, bool all = false) const override
    {
        return isCompatible(other, all);
    }

    /// \copydoc Type::isCompatible
    virtual bool isCompatible(const Type &other, bool all) const override;

    /// \returns true if this is a superstructure of \p other,
    /// i.e. we have the same types at the same offsets as \p other
    bool isSuperStructOf(const SharedConstType &other) const;

    /// \returns true if this is a substructure of other,
    /// i.e. other has the same types at the same offsets as this
    bool isSubStructOf(const SharedConstType &other) const;

    /// Append a new member variable to this struct/class.
    /// \param memberType the type of the new member variable.
    /// \param memberName the new name of the member variable.
    void addMember(SharedType memberType, const QString &memberName);

    /// \returns the number of member variables in this structure type
    int getNumMembers() const { return m_types.size(); }

    SharedType getMemberTypeByIdx(int idx);
    SharedType getMemberTypeByName(const QString &name);
    SharedType getMemberTypeByOffset(unsigned offsetInBits);

    QString getMemberNameByIdx(int idx);
    QString getMemberNameByOffset(size_t offsetInBits);

    unsigned getMemberOffsetByIdx(int idx);
    unsigned getMemberOffsetByName(const QString &name);

    void setMemberTypeByOffset(unsigned offsetInBits, SharedType ty);
    void setMemberNameByOffset(unsigned offsetInBits, const QString &name);

    unsigned getOffsetRemainder(unsigned n);

private:
    std::vector<SharedType> m_types;
    std::vector<QString> m_names;
};
