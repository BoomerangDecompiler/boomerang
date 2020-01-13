#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "FuncType.h"

#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/type/SizeType.h"


FuncType::FuncType(const std::shared_ptr<Signature> &sig)
    : Type(TypeClass::Func)
    , m_signature(sig)
{
}


FuncType::~FuncType()
{
}


std::shared_ptr<FuncType> FuncType::get(const std::shared_ptr<Signature> &sig)
{
    return std::make_shared<FuncType>(sig);
}


SharedType FuncType::clone() const
{
    return FuncType::get(m_signature ? m_signature->clone() : nullptr);
}


Type::Size FuncType::getSize() const
{
    return 0; /* always nagged me */
}


bool FuncType::operator==(const Type &other) const
{
    if (!other.isFunc()) {
        return false;
    }

    const FuncType &otherFunc = static_cast<const FuncType &>(other);

    // Note: some functions don't have a signature (e.g. indirect calls that have not yet been
    // successfully analysed)
    if (m_signature.get() != otherFunc.getSignature()) {
        return false;
    }

    return m_signature ? *m_signature == *otherFunc.m_signature : true;
}


bool FuncType::operator<(const Type &other) const
{
    if (getId() != other.getId()) {
        return getId() < other.getId();
    }

    // Note: Functions without signatures are less than functions with signatures
    const FuncType &otherFunc = static_cast<const FuncType &>(other);
    if (m_signature) {
        return otherFunc.m_signature ? *m_signature < *otherFunc.m_signature : false;
    }
    else {
        return otherFunc.getSignature() != nullptr;
    }
}


QString FuncType::getCtype(bool final) const
{
    if (m_signature == nullptr) {
        return "void (void)";
    }

    QString s;

    if (m_signature->getNumReturns() == 0) {
        s += "void";
    }
    else {
        s += m_signature->getReturnType(0)->getCtype(final);
    }

    s += " (";

    for (int i = 0; i < m_signature->getNumParams(); i++) {
        if (i != 0) {
            s += ", ";
        }

        s += m_signature->getParamType(i)->getCtype(final);
    }

    s += ")";
    return s;
}


void FuncType::getReturnAndParam(QString &ret, QString &param)
{
    if (m_signature == nullptr) {
        ret   = "void";
        param = "(void)";
        return;
    }

    if (m_signature->getNumReturns() == 0) {
        ret = "void";
    }
    else {
        ret = m_signature->getReturnType(0)->getCtype();
    }

    QString s;
    s += " (";

    for (int i = 0; i < m_signature->getNumParams(); i++) {
        if (i != 0) {
            s += ", ";
        }

        s += m_signature->getParamType(i)->getCtype();
    }

    s += ")";
    param = s;
}


SharedType FuncType::meetWith(SharedType other, bool &changed, bool useHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return const_cast<FuncType *>(this)->shared_from_this();
    }

    // NOTE: at present, compares names as well as types and num parameters
    if (*this == *other) {
        return const_cast<FuncType *>(this)->shared_from_this();
    }

    return createUnion(other, changed, useHighestPtr);
}


bool FuncType::isCompatible(const Type &other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (*this == other) {
        return true; // MVE: should not compare names!
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToSize() && (static_cast<const SizeType &>(other).getSize() == STD_SIZE)) {
        return true;
    }

    if (other.resolvesToFunc()) {
        if (getSignature() && other.as<FuncType>()->getSignature()) {
            return *getSignature() == *other.as<FuncType>()->getSignature();
        }
        else if (getSignature() == other.as<FuncType>()->getSignature()) {
            return true;
        }
    }

    return false;
}
