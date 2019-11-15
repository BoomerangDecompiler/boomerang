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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/util/Util.h"

#include <vector>


/**
 * Base class for all nodes/vertices in a directed graph.
 */
template<typename Derived>
class BOOMERANG_API GraphNode
{
public:
    inline int getNumPredecessors() const { return m_predecessors.size(); }
    inline int getNumSuccessors() const { return m_successors.size(); }

    /// \returns all predecessors of this BB.
    const std::vector<Derived *> &getPredecessors() const { return m_predecessors; }

    /// \returns all successors of this BB.
    const std::vector<Derived *> &getSuccessors() const { return m_successors; }

    /// \returns the \p i-th predecessor of this BB.
    /// Returns nullptr if \p i is out of range.
    Derived *getPredecessor(int i)
    {
        return Util::inRange(i, 0, getNumPredecessors()) ? m_predecessors[i] : nullptr;
    }

    /// \returns the \p i-th predecessor of this BB.
    /// Returns nullptr if \p i is out of range.
    const Derived *getPredecessor(int i) const
    {
        return Util::inRange(i, 0, getNumPredecessors()) ? m_predecessors[i] : nullptr;
    }

    /// \returns the \p i-th successor of this BB.
    /// Returns nullptr if \p i is out of range.
    Derived *getSuccessor(int i)
    {
        return Util::inRange(i, 0, getNumSuccessors()) ? m_successors[i] : nullptr;
    }

    const Derived *getSuccessor(int i) const
    {
        return Util::inRange(i, 0, getNumSuccessors()) ? m_successors[i] : nullptr;
    }

    /// Change the \p i-th predecessor of this BB.
    /// \param i index (0-based)
    void setPredecessor(int i, Derived *predecessor)
    {
        assert(Util::inRange(i, 0, getNumPredecessors()));
        m_predecessors[i] = predecessor;
    }

    /// Change the \p i-th successor of this BB.
    /// \param i index (0-based)
    void setSuccessor(int i, Derived *successor)
    {
        assert(Util::inRange(i, 0, getNumSuccessors()));
        m_successors[i] = successor;
    }

    /// Add a predecessor to this BB.
    void addPredecessor(Derived *predecessor) { m_predecessors.push_back(predecessor); }

    /// Add a successor to this BB.
    void addSuccessor(Derived *successor) { m_successors.push_back(successor); }

    /// Remove a predecessor BB.
    void removePredecessor(Derived *pred)
    {
        // Only remove a single predecessor (prevents issues with double edges)
        for (auto it = m_predecessors.begin(); it != m_predecessors.end(); ++it) {
            if (*it == pred) {
                m_predecessors.erase(it);
                return;
            }
        }
    }

    /// Remove a successor BB
    void removeSuccessor(Derived *succ)
    {
        // Only remove a single successor (prevents issues with double edges)
        for (auto it = m_successors.begin(); it != m_successors.end(); ++it) {
            if (*it == succ) {
                m_successors.erase(it);
                return;
            }
        }
    }

    /// Removes all successor BBs.
    /// Called when noreturn call is found
    void removeAllSuccessors() { m_successors.clear(); }

    /// removes all predecessor BBs.
    void removeAllPredecessors() { m_predecessors.clear(); }

    /// \returns true if this BB is a (direct) predecessor of \p bb,
    /// i.e. there is an edge from this BB to \p bb
    bool isPredecessorOf(const Derived *bb) const
    {
        return std::find(m_successors.begin(), m_successors.end(), bb) != m_successors.end();
    }

    /// \returns true if this BB is a (direct) successor of \p bb,
    /// i.e. there is an edge from \p bb to this BB.
    bool isSuccessorOf(const Derived *bb) const
    {
        return std::find(m_predecessors.begin(), m_predecessors.end(), bb) != m_predecessors.end();
    }

private:
    /* in-edges and out-edges */
    std::vector<Derived *> m_predecessors; ///< Vector of in-edges
    std::vector<Derived *> m_successors;   ///< Vector of out-edges
};
