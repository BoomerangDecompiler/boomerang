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

#include <algorithm>
#include <utility>


/**
 * Class for right-open intervals.
 */
template<typename T>
class Interval
{
public:
    /// Construct a right-open interval
    explicit Interval(const T &_lower, const T &_upper)
        : m_lower(_lower)
        , m_upper(_upper)
    {
    }

    ~Interval() = default;

    Interval(const Interval<T> &other)
        : m_lower(other.m_lower)
        , m_upper(other.m_upper)
    {
    }

    Interval(Interval<T> &&other)
        : m_lower(std::move(other.m_lower))
        , m_upper(std::move(other.m_upper))
    {
    }

    const Interval<T> &operator=(const Interval<T> &other)
    {
        m_lower(other.lower());
        m_upper(other.upper());
        return *this;
    }

    const Interval<T> &operator=(Interval<T> &&other)
    {
        m_lower = std::move(other.m_lower);
        m_upper = std::move(other.m_upper);
        return *this;
    }

public:
    /// \returns the lower bound of the interval.
    inline const T &lower() const { return m_lower; }
    /// \returns the upper bound of the interval.
    inline const T &upper() const { return m_upper; }

    /**
     * Checks if \p value is contained in this interval.
     * Returns false for the upper bound (since it is a right-open interval).
     */
    inline bool contains(const T &value) const { return m_lower <= value && m_upper > value; }

    /**
     * \returns true if this and the other interval are adjacent
     * or partially contained within each other.
     * i.e. there is no "hole" between the two intervals.
     */
    inline bool canMergeWith(const Interval<T> &other) const
    {
        return other.lower() <= upper() && other.upper() >= lower();
    }

    /// \returns whether the other interval is fully contained within this
    /// interval, i.e. other - *this == empty
    inline bool containsInterval(const Interval<T> &other) const
    {
        return lower() <= other.lower() && upper() >= other.upper();
    }

    /**
     * Compare this interval to another interval by its lower bound.
     * \returns whether this->lower() < other.lower()
     */
    inline bool operator<(const Interval<T> &other) const { return m_lower < other.m_lower; }

    /**
     * Compare the lower bound of this interval to a single value.
     * \returns whether this->lower() < value
     */
    inline bool operator<(const T &value) const { return m_lower < value; }

    const Interval<T> &operator+=(const Interval<T> &other)
    {
        assert(this->canMergeWith(other));
        m_lower = std::min(lower(), other.lower());
        m_upper = std::max(upper(), other.upper());
        return *this;
    }

private:
    T m_lower; ///< Lower bound of the interval
    T m_upper; ///< Upper bound of the interval
};


/**
 * Merge two intervals.
 */
template<typename T>
Interval<T> operator+(const Interval<T> &first, const Interval<T> &second)
{
    return Interval<T>(first) += second;
}
