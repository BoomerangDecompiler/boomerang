#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Range.h"


Range::Range()
    : m_stride(1)
    , m_lowerBound(MIN)
    , m_upperBound(MAX)
{
    m_base = Const::get(0);
}


Range::Range(int _stride, int _lowerBound, int _upperBound, SharedExp _base)
    : m_stride(_stride)
    , m_lowerBound(_lowerBound)
    , m_upperBound(_upperBound)
    , m_base(_base)
{
    if ((m_lowerBound == m_upperBound) && (m_lowerBound == 0) && ((m_base->getOper() == opMinus) || (m_base->getOper() == opPlus)) &&
        m_base->getSubExp2()->isIntConst()) {
        this->m_lowerBound = m_base->access<Const, 2>()->getInt();

        if (m_base->getOper() == opMinus) {
            this->m_lowerBound = -this->m_lowerBound;
        }

        this->m_upperBound = this->m_lowerBound;
        this->m_base       = m_base->getSubExp1();
    }
    else {
        if (m_base == nullptr) {
            // NOTE: was "base = Const::get(0);"
            this->m_base = Const::get(0);
        }

        if (m_lowerBound > m_upperBound) {
            this->m_upperBound = m_lowerBound;
        }

        if (m_upperBound < m_lowerBound) {
            this->m_lowerBound = m_upperBound;
        }
    }
}


QString Range::toString() const
{
    QString     res;
    QTextStream os(&res);

    assert(m_lowerBound <= m_upperBound);

    if (m_base->isIntConst() && (m_base->access<Const>()->getInt() == 0) && (m_lowerBound == MIN) && (m_upperBound == MAX)) {
        os << "T";
        return res;
    }

    bool needPlus = false;

    if (m_lowerBound == m_upperBound) {
        if (!m_base->isIntConst() || (m_base->access<Const>()->getInt() != 0)) {
            if (m_lowerBound != 0) {
                os << m_lowerBound;
                needPlus = true;
            }
        }
        else {
            needPlus = true;
            os << m_lowerBound;
        }
    }
    else {
        if (m_stride != 1) {
            os << m_stride;
        }

        os << "[";

        if (m_lowerBound == MIN) {
            os << "-inf";
        }
        else {
            os << m_lowerBound;
        }

        os << ", ";

        if (m_upperBound == MAX) {
            os << "inf";
        }
        else {
            os << m_upperBound;
        }

        os << "]";
        needPlus = true;
    }

    if (!m_base->isIntConst() || (m_base->access<Const>()->getInt() != 0)) {
        if (needPlus) {
            os << " + ";
        }

        m_base->print(os);
    }

    return res;
}


void Range::unionWith(Range& r)
{
    if (DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("unioning %1 with %2 got...", toString(), r);
    }

    assert(m_base && r.m_base);

    if ((m_base->getOper() == opMinus) && (r.m_base->getOper() == opMinus) && (*m_base->getSubExp1() == *r.m_base->getSubExp1()) &&
        m_base->getSubExp2()->isIntConst() && r.m_base->getSubExp2()->isIntConst()) {
        int c1 = m_base->access<Const, 2>()->getInt();
        int c2 = r.m_base->access<Const, 2>()->getInt();

        if (c1 != c2) {
            if ((m_lowerBound == r.m_lowerBound) && (m_upperBound == r.m_upperBound) && (m_lowerBound == 0)) {
                m_lowerBound = std::min(-c1, -c2);
                m_upperBound = std::max(-c1, -c2);
                m_base       = m_base->getSubExp1();

                if (DEBUG_RANGE_ANALYSIS) {
                    LOG_VERBOSE("%1", toString());
                }

                return;
            }
        }
    }

    if (!(*m_base == *r.m_base)) {
        m_stride     = 1;
        m_lowerBound = MIN;
        m_upperBound = MAX;
        m_base       = Const::get(0);

        if (DEBUG_RANGE_ANALYSIS) {
            LOG_VERBOSE("%1", toString());
        }

        return;
    }

    if (m_stride != r.m_stride) {
        m_stride = std::min(m_stride, r.m_stride);
    }

    if (m_lowerBound != r.m_lowerBound) {
        m_lowerBound = std::min(m_lowerBound, r.m_lowerBound);
    }

    if (m_upperBound != r.m_upperBound) {
        m_upperBound = std::max(m_upperBound, r.m_upperBound);
    }

    if (VERBOSE && DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("%1", toString());
    }
}


void Range::widenWith(Range& r)
{
    if (VERBOSE && DEBUG_RANGE_ANALYSIS) {
        LOG_VERBOSE("Widening %1 with %2 got...", toString(), r);
    }

    if (!(*m_base == *r.m_base)) {
        m_stride     = 1;
        m_lowerBound = MIN;
        m_upperBound = MAX;
        m_base       = Const::get(0);

        if (VERBOSE && DEBUG_RANGE_ANALYSIS) {
            LOG_VERBOSE("%1", toString());
        }

        return;
    }

    // ignore stride for now
    if (r.getLowerBound() < m_lowerBound) {
        m_lowerBound = MIN;
    }

    if (r.getUpperBound() > m_upperBound) {
        m_upperBound = MAX;
    }

    if (DEBUG_RANGE_ANALYSIS) {
        LOG_MSG(this->toString());
    }
}


bool Range::operator==(Range& other)
{
    return m_stride == other.m_stride && m_lowerBound == other.m_lowerBound && m_upperBound == other.m_upperBound &&
           *m_base == *other.m_base;
}
