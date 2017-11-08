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


#include "boomerang/util/Util.h"

using SharedExp = std::shared_ptr<class Exp>;


/**
 *
 */
class Range : public Printable
{
public:
    typedef int value_type;

protected:
    value_type m_stride;
    value_type m_lowerBound;
    value_type m_upperBound;
    SharedExp m_base;

public:
    Range();
    Range(int stride, int lowerBound, int upperBound, SharedExp base);

    SharedExp getBase() const { return m_base; }
    int getStride() const { return m_stride; }
    int getLowerBound() const { return m_lowerBound; }
    int getUpperBound() const { return m_upperBound; }
    void unionWith(Range& r);
    void widenWith(Range& r);
    QString toString() const override;
    bool operator==(Range& other);

    static const int MAX = INT32_MAX;
    static const int MIN = INT32_MIN;
};
