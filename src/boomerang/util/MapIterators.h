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


#include <iterator>


template<typename M>
class MapValueIterator
{
public:
    typedef typename M::mapped_type value_type;
    typedef typename M::mapped_type &reference;
    typedef typename M::mapped_type *pointer;

    typedef typename M::iterator::iterator_category iterator_category;
    typedef typename M::iterator::difference_type difference_type;

public:
    MapValueIterator(typename M::iterator it)
        : m_it(it)
    {
    }

    MapValueIterator(const MapValueIterator<M> &it) { this->m_it = it.m_it; }

    MapValueIterator<M> &operator=(const MapValueIterator<M> &it)
    {
        this->m_it = it.m_it;
        return *this;
    }

    bool operator==(const MapValueIterator<M> &it) const { return m_it == it.m_it; }
    bool operator!=(const MapValueIterator<M> &it) const { return m_it != it.m_it; }

    reference operator*() const { return m_it->second; }
    pointer operator->() const { return &m_it->second; }

    MapValueIterator<M> &operator++()
    {
        ++m_it;
        return *this;
    }

    MapValueIterator<M> &operator--()
    {
        --m_it;
        return *this;
    }

    MapValueIterator<M> operator++(int) { return MapValueIterator<M>(m_it++); }
    MapValueIterator<M> operator--(int) { return MapValueIterator<M>(m_it--); }

private:
    typename M::iterator m_it;
};


template<typename M>
class MapValueConstIterator
{
public:
    typedef const typename M::mapped_type value_type;
    typedef const typename M::mapped_type &reference;
    typedef const typename M::mapped_type *pointer;

    typedef typename M::const_iterator::iterator_category iterator_category;
    typedef typename M::const_iterator::difference_type difference_type;

public:
    MapValueConstIterator(typename M::const_iterator it)
        : m_it(it)
    {
    }
    MapValueConstIterator(const MapValueConstIterator<M> &it) { this->m_it = it.m_it; }

    MapValueConstIterator<M> &operator=(const MapValueConstIterator<M> &it)
    {
        this->m_it = it->m_it;
        return *this;
    }

    bool operator==(const MapValueConstIterator<M> &it) const { return m_it == it.m_it; }
    bool operator!=(const MapValueConstIterator<M> &it) const { return m_it != it.m_it; }

    reference operator*() const { return m_it->second; }
    pointer operator->() const { return &m_it->second; }

    MapValueConstIterator<M> &operator++()
    {
        ++m_it;
        return *this;
    }

    MapValueConstIterator<M> &operator--()
    {
        --m_it;
        return *this;
    }

    MapValueConstIterator<M> operator++(int) { return MapValueConstIterator<M>(m_it++); }
    MapValueConstIterator<M> operator--(int) { return MapValueConstIterator<M>(m_it--); }

private:
    typename M::const_iterator m_it;
};


template<typename M>
using MapValueReverseIterator = std::reverse_iterator<MapValueIterator<M>>;
template<typename M>
using MapValueConstReverseIterator = std::reverse_iterator<MapValueConstIterator<M>>;
