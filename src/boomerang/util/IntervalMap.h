#pragma once

#include <map>

/**
 * Right open interval
 */
template<typename T>
class Interval
{
public:
    Interval(const T& lower, const T& upper)
        : m_lower(lower)
        , m_upper(upper)
    {}
    ~Interval() = default;

    const T& lower() const { return m_lower; }
    const T& upper() const { return m_upper; }

    bool isContained(const T& value) const
    {
        return m_lower <= value && m_upper > value;
    }

    bool operator<(const Interval<T>& other) const
    {
        return m_lower < other.m_lower;
    }

    bool operator<(const T& value) const
    {
        return m_lower < value;
    }

private:
    T m_lower; ///< Lower bound of the interval
    T m_upper; ///< Upper bound of the interval
};


/**
 * A map that maps intervals of Key types to Value types.
 * Intervals may overlap each other.
 */
template<typename Key, typename Value>
class IntervalMap
{
public:
    typedef typename std::map<Interval<Key>, Value> Data;

    typedef typename Data::iterator iterator;
    typedef typename Data::const_iterator const_iterator;
    typedef typename Data::reverse_iterator reverse_iterator;
    typedef typename Data::const_reverse_iterator const_reverse_iterator;

public:
    IntervalMap() {}
    ~IntervalMap() = default;

    bool isEmpty() const { return m_data.empty(); }

    void clear() { m_data.clear(); }

    iterator insert(const Interval<Key>& key, const Value& value)
    {
        std::pair<typename Data::iterator, bool> p = m_data.insert({ key, value });
        return p.second ? p.first : m_data.end();
    }

    iterator insert(const Key& lower, const Key& upper, const Value& value)
    {
        return insert(Interval<Key>(lower, upper), value);
    }

    const_iterator find(const Key& key) const
    {
        // todo: speed up
        for (const_iterator it = begin(); it != end(); it++) {
            const Key& lower = it->first.lower();
            const Key& upper = it->first.upper();

            if (upper <= key) {
                continue;
            }
            else if (lower > key) {
                break;
            }
            else {
                assert(it->first.isContained(key));
                return it;
            }
        }
        return end();
    }

    /**
     * Returns an iterator range containng all intervals between \p lower and \p upper
     */
    std::pair<const_iterator, const_iterator> equalRange(const Key& lower, const Key& upper)
    {
        return equalRange(Interval<Key>(lower, upper));
    }

    std::pair<const_iterator, const_iterator> equalRange(const Interval<Key>& interval)
    {
        const_iterator itLower = end();
        const_iterator itUpper = end();

        // todo: speed up
        for (const_iterator it = begin(); it != end(); it++) {
            if (it->first.upper() > interval.lower() && itLower == end()) {
                itLower = it;
            }

            if (itLower != end() && it->first.lower() <= interval.upper()) {
                itUpper = it;
            }
        }

        return std::make_pair(itLower, itUpper);
    }

    iterator begin() { return m_data.begin(); }
    iterator end()   { return m_data.end(); }
    const_iterator begin() const { return m_data.begin(); }
    const_iterator end()   const { return m_data.end(); }
    reverse_iterator rbegin() { return m_data.rbegin(); }
    reverse_iterator rend()   { return m_data.rend();   }
    const_reverse_iterator rbegin() const { return m_data.rbegin(); }
    const_reverse_iterator rend() const { return m_data.rend(); }

private:
    std::map<Interval<Key>, Value, std::less<Interval<Key>>> m_data;
};
