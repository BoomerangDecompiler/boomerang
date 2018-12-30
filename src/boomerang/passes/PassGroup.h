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


#include <QString>

#include <vector>


class IPass;


/// PassGroups are immutable aggregations of Passes with a name.
/// The passes contained within a single pass group are executed sequentially on a single UserProc.
class PassGroup
{
    typedef std::vector<IPass *> Passes;
    typedef Passes::const_iterator const_iterator;

public:
    explicit PassGroup(const QString &name, const std::initializer_list<IPass *> &passes);

    const_iterator begin() const { return m_passes.begin(); }
    const_iterator end() const { return m_passes.end(); }

    const QString &getName() const { return m_name; }

private:
    QString m_name;
    Passes m_passes;
};
