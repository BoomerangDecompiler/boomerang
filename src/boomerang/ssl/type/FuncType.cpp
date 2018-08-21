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
    , signature(sig)
{
}


FuncType::~FuncType()
{
}


SharedType FuncType::clone() const
{
    return FuncType::get(signature);
}


size_t FuncType::getSize() const
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
    if (!signature) {
        return otherFunc.signature == nullptr;
    }

    return *signature == *otherFunc.signature;
}


bool FuncType::operator<(const Type &other) const
{
    if (id != other.getId()) {
        return id < other.getId();
    }

    // FIXME: Need to compare signatures
    return true;
}


QString FuncType::getCtype(bool final) const
{
    if (signature == nullptr) {
        return "void (void)";
    }

    QString s;

    if (signature->getNumReturns() == 0) {
        s += "void";
    }
    else {
        s += signature->getReturnType(0)->getCtype(final);
    }

    s += " (";

    for (int i = 0; i < signature->getNumParams(); i++) {
        if (i != 0) {
            s += ", ";
        }

        s += signature->getParamType(i)->getCtype(final);
    }

    s += ")";
    return s;
}


void FuncType::getReturnAndParam(QString &ret, QString &param)
{
    if (signature == nullptr) {
        ret   = "void";
        param = "(void)";
        return;
    }

    if (signature->getNumReturns() == 0) {
        ret = "void";
    }
    else {
        ret = signature->getReturnType(0)->getCtype();
    }

    QString s;
    s += " (";

    for (int i = 0; i < signature->getNumParams(); i++) {
        if (i != 0) {
            s += ", ";
        }

        s += signature->getParamType(i)->getCtype();
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
    assert(signature);

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
        assert(other.as<FuncType>()->signature);

        if (*other.as<FuncType>()->signature == *signature) {
            return true;
        }
    }

    return false;
}
