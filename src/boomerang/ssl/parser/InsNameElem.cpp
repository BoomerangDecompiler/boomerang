#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "InsNameElem.h"

#include "boomerang/util/Types.h"

#include <cassert>
#include <map>
#include <string>


InsNameElem::InsNameElem(const QString &name)
    : m_elemName(name)
{
}


InsNameElem::~InsNameElem()
{
}


size_t InsNameElem::getNumTokens() const
{
    return 1;
}


QString InsNameElem::getInstruction() const
{
    return (m_nextElem != nullptr) ? (m_elemName + m_nextElem->getInstruction()) : m_elemName;
}


QString InsNameElem::getInsPattern() const
{
    return (m_nextElem != nullptr) ? (m_elemName + m_nextElem->getInsPattern()) : m_elemName;
}


void InsNameElem::getRefMap(std::map<QString, std::shared_ptr<InsNameElem>> &map)
{
    if (m_nextElem != nullptr) {
        m_nextElem->getRefMap(map);
    }
    else {
        map.clear();
    }
}


int InsNameElem::getNumInstructions() const
{
    return (m_nextElem != nullptr) ? (m_nextElem->getNumInstructions() * getNumTokens())
                                   : getNumTokens();
}


void InsNameElem::append(std::shared_ptr<InsNameElem> next)
{
    if (m_nextElem == nullptr) {
        m_nextElem = next;
    }
    else {
        m_nextElem->append(next);
    }
}


bool InsNameElem::increment()
{
    if ((m_nextElem == nullptr) || m_nextElem->increment()) {
        m_value++;
    }

    if (m_value >= getNumTokens()) {
        m_value = 0;
        return true;
    }

    return false;
}


void InsNameElem::reset()
{
    m_value = 0;

    if (m_nextElem != nullptr) {
        m_nextElem->reset();
    }
}


int InsNameElem::getValue(void) const
{
    return m_value;
}


InsOptionElem::InsOptionElem(const QString &name)
    : InsNameElem(name)
{
}


size_t InsOptionElem::getNumTokens() const
{
    return 2;
}


QString InsOptionElem::getInstruction() const
{
    QString s = (m_nextElem != nullptr)
                    ? ((getValue() == 0) ? (m_elemName + m_nextElem->getInstruction())
                                         : m_nextElem->getInstruction())
                    : ((getValue() == 0) ? m_elemName : "");

    return s;
}


QString InsOptionElem::getInsPattern() const
{
    return (m_nextElem != nullptr) ? ('\'' + m_elemName + '\'' + m_nextElem->getInsPattern())
                                   : ('\'' + m_elemName + '\'');
}


InsListElem::InsListElem(const QString &name, const std::shared_ptr<Table> &t, const QString &idx)
    : InsNameElem(name)
    , m_indexName(idx)
    , m_theTable(t)
{
}


size_t InsListElem::getNumTokens() const
{
    return m_theTable->getRecords().size();
}


QString InsListElem::getInstruction() const
{
    return (m_nextElem != nullptr)
               ? (m_theTable->getRecords()[getValue()] + m_nextElem->getInstruction())
               : m_theTable->getRecords()[getValue()];
}


QString InsListElem::getInsPattern() const
{
    return (m_nextElem != nullptr)
               ? (m_elemName + '[' + m_indexName + ']' + m_nextElem->getInsPattern())
               : (m_elemName + '[' + m_indexName + ']');
}


void InsListElem::getRefMap(std::map<QString, std::shared_ptr<InsNameElem>> &m)
{
    if (m_nextElem != nullptr) {
        m_nextElem->getRefMap(m);
    }
    else {
        m.clear();
    }

    m[m_indexName] = shared_from_this();
    // of course, we're assuming that we've already checked (try in the parser)
    // that indexname hasn't been used more than once on this line ..
}


QString InsListElem::getIndex() const
{
    return m_indexName;
}
