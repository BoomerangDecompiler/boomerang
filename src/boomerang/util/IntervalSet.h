#pragma once

#include "boomerang/util/Interval.h"

#include <set>

/**
 * An IntervalSet implements a set as a set of intervals, merging adjoining intervals.
 */
template<typename T>
class IntervalSet
{
public:
    typedef typename std::set<Interval<T>, std::less<Interval<T>>> Data;

    typedef typename Data::iterator iterator;
    typedef typename Data::const_iterator const_iterator;
    typedef typename Data::reverse_iterator reverse_iterator;
    typedef typename Data::const_reverse_iterator const_reverse_iterator;

public:
    IntervalSet() {}
    ~IntervalSet() = default;

public:
    /// \returns true if the set does not contain any elements.
    bool isEmpty() const { return m_data.empty(); }

    /// Removes all intervals from the set.
    void clear() { m_data.clear(); }

    iterator insert(const T& from, const T& to) { return insert(Interval<T>(from, to)); }
    iterator insert(const Interval<T>& interval)
    {
        typename Data::iterator firstInRange, lastInRange;
        std::tie(firstInRange, lastInRange) = equalRange(interval.lower(), interval.upper());

        T minLower = std::min(interval.lower(), firstInRange->lower());
        T maxUpper = std::max(interval.upper(), lastInRange->upper());

        typename Data::iterator it = m_data.erase(firstInRange, lastInRange);
        return m_data.insert(it, Interval<T>(minLower, maxUpper));
    }

    iterator begin() { return m_data.begin(); }
    iterator end()   { return m_data.end(); }
    const_iterator begin() const { return m_data.begin(); }
    const_iterator end()   const { return m_data.end(); }
    reverse_iterator rbegin() { return m_data.rbegin(); }
    reverse_iterator rend()   { return m_data.rend();   }
    const_reverse_iterator rbegin() const { return m_data.rbegin(); }
    const_reverse_iterator rend() const { return m_data.rend(); }


    /**
     * Returns an iterator range containing all intervals between \p lower and \p upper
     */
    std::pair<iterator, iterator> equalRange(const T& lower, const T& upper)
    {
        return equalRange(Interval<T>(lower, upper));
    }

    std::pair<iterator, iterator> equalRange(const Interval<T>& interval)
    {
        typename Data::iterator itLower = end();
        typename Data::iterator itUpper = end();

        // todo: speed up
        for (iterator it = begin(); it != end(); it++) {
            if (itLower == end() && it->upper() > interval.lower()) {
                itLower = it;
            }

            if (itLower != end()) {
                if (it->lower() < interval.upper()) {
                    itUpper = it;
                }
                else {
                    break; // not in range any more
                }
            }
        }

        return std::make_pair(itLower, itUpper);
    }

    /// \returns true if \p value is contained in any interval of this set.
    bool isContained(const T& value) const
    {
        if (isEmpty()) {
            return false;
        }

        const_iterator it = std::lower_bound(m_data.begin(), m_data.end(), value);
        if (it != end() && it->isContained(value)) {
            return true;
        }
        else if (it == m_data.begin()) {
            return it->isContained(value); // cannot do std::prev(begin());
        }
        else {
            // we know it exists since the set is not empty
            it = std::prev(it);
            return it->isContained(value);
        }
    }

private:
    Data m_data;
};
