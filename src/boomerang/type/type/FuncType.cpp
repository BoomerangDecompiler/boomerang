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


#include "boomerang/db/Signature.h"


FuncType::FuncType(const std::shared_ptr<Signature>& sig)
    : Type(eFunc)
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


bool FuncType::operator==(const Type& other) const
{
    if (!other.isFunc()) {
        return false;
    }

    // Note: some functions don't have a signature (e.g. indirect calls that have not yet been successfully analysed)

    if (signature == nullptr) {
        return ((FuncType&)other).signature == nullptr;
    }

    return *signature == *((FuncType&)other).signature;
}


bool FuncType::operator<(const Type& other) const
{
    if (id < other.getId()) {
        return true;
    }

    if (id > other.getId()) {
        return false;
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

    for (unsigned i = 0; i < signature->getNumParams(); i++) {
        if (i != 0) {
            s += ", ";
        }

        s += signature->getParamType(i)->getCtype(final);
    }

    s += ")";
    return s;
}


void FuncType::getReturnAndParam(QString& ret, QString& param)
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

    for (unsigned i = 0; i < signature->getNumParams(); i++) {
        if (i != 0) {
            s += ", ";
        }

        s += signature->getParamType(i)->getCtype();
    }

    s    += ")";
    param = s;
}
