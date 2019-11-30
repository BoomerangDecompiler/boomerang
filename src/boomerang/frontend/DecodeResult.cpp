#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DecodeResult.h"


DecodeResult::DecodeResult()
{
    reset();
}


DecodeResult::DecodeResult(DecodeResult &&other)
    : m_rtls(std::move(other.m_rtls))
    , reLift(std::move(other.reLift))
{
}


DecodeResult::~DecodeResult()
{
}


DecodeResult &DecodeResult::operator=(DecodeResult &&other)
{
    m_rtls = std::move(other.m_rtls);
    reLift = std::move(other.reLift);

    return *this;
}


void DecodeResult::reset()
{
    m_rtls.clear();
    reLift = false;
}


void DecodeResult::fillRTL(std::unique_ptr<RTL> _rtl)
{
    assert(m_rtls.empty());
    m_rtls.push_back(std::move(_rtl));
}


std::unique_ptr<RTL> DecodeResult::useRTL()
{
    assert(!m_rtls.empty());
    std::unique_ptr<RTL> rtl = std::move(m_rtls.front());
    m_rtls.clear();
    return rtl;
}


RTL *DecodeResult::getRTL()
{
    return !m_rtls.empty() ? m_rtls.front().get() : nullptr;
}


const RTL *DecodeResult::getRTL() const
{
    return !m_rtls.empty() ? m_rtls.front().get() : nullptr;
}
