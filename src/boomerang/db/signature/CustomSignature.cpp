#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CustomSignature.h"


#include "boomerang/ssl/exp/Location.h"
#include "boomerang/util/Util.h"


CustomSignature::CustomSignature(const QString& name)
    : Signature(name)
    , m_spReg(0)
{
}


std::shared_ptr<Signature> CustomSignature::clone() const
{
    auto result = std::make_shared<CustomSignature>(m_name);

    Util::clone(m_params, result->m_params);
    Util::clone(m_returns, result->m_returns);

    result->m_ellipsis        = m_ellipsis;
    result->m_spReg           = m_spReg;
    result->m_forced          = m_forced;
    result->m_preferredName   = m_preferredName;
    result->m_unknown         = m_unknown;
    result->m_sigFile         = m_sigFile;

    return result;
}


void CustomSignature::setSP(int spReg)
{
    m_spReg = spReg;

    if (m_spReg) {
        addReturn(Location::regOf(m_spReg));
        // addImplicitParameter(PointerType::get(new IntegerType()), "sp",
        //                            Location::regOf(sp), nullptr);
    }
}
