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

    ~CompoundType() override;

    CompoundType &operator=(CompoundType &other) = default;
    CompoundType &operator=(CompoundType &&other) = default;

public:
    static std::shared_ptr<CompoundType> get() { return std::make_shared<CompoundType>(); }

    /// \copydoc Type::operator==
    bool operator==(const Type &other) const override;

    /// \copydoc Type::operator<
    bool operator<(const Type &other) const override;

    /// \copydoc Type::clone
    SharedType clone() const override;

    /// \copydoc Type::getSize
    Size getSize() const override;

    /// \copydoc Type::getCtype
    QString getCtype(bool final = false) const override;

    /// \copydoc Type::isCompatibleWith
    bool isCompatibleWith(const Type &other, bool all = false) const override;

    /// \copydoc Type::meetWith
    SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

public:
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
    SharedType getMemberTypeByOffset(uint64 offsetInBits);

    QString getMemberNameByIdx(int idx);
    QString getMemberNameByOffset(uint64 offsetInBits);

    uint64 getMemberOffsetByIdx(int idx);
    uint64 getMemberOffsetByName(const QString &name);

    void setMemberTypeByOffset(uint64 offsetInBits, SharedType ty);
    void setMemberNameByOffset(uint64 offsetInBits, const QString &name);

    uint64 getOffsetRemainder(uint64 bitOffset);

protected:
    /// \copydoc Type::isCompatible
    bool isCompatible(const Type &other, bool all) const override;

private:
    std::vector<SharedType> m_types;
    std::vector<QString> m_names;
};
