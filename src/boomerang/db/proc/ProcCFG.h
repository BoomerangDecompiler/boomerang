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


#include "boomerang/db/IRFragment.h"
#include "boomerang/frontend/MachineInstruction.h"
#include "boomerang/ssl/exp/ExpHelp.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/Address.h"
#include "boomerang/util/MapIterators.h"
#include "boomerang/util/Util.h"

#include <list>
#include <map>
#include <memory>
#include <set>


class Function;
class UserProc;
class BasicBlock;
class RTL;
class Parameter;

using RTLList = std::list<std::unique_ptr<RTL>>;

enum class BBType;


/// Contains all the IRFragment objects for a single UserProc.
/// These fragments contain all the RTLs for the procedure, so by traversing the CFG,
/// one traverses the IR for the whole procedure.
class BOOMERANG_API ProcCFG
{
    // FIXME order is undefined if two fragments come from the same BB
    typedef std::multiset<IRFragment *, Util::ptrCompare<IRFragment>> FragmentSet;
    typedef std::map<SharedConstExp, SharedStmt, lessExpStar> ExpStatementMap;

public:
    typedef FragmentSet::iterator iterator;
    typedef FragmentSet::const_iterator const_iterator;
    typedef FragmentSet::reverse_iterator reverse_iterator;
    typedef FragmentSet::const_reverse_iterator const_reverse_iterator;

public:
    /// Creates an empty CFG for the function \p proc
    ProcCFG(UserProc *proc);
    ProcCFG(const ProcCFG &other) = delete;
    ProcCFG(ProcCFG &&other)      = default;

    ~ProcCFG();

    ProcCFG &operator=(const ProcCFG &other) = delete;
    ProcCFG &operator=(ProcCFG &&other) = default;

public:
    /// \note When removing a fragment, the iterator(s) pointing to the removed fragment
    /// are invalidated.
    iterator begin() { return iterator(m_fragmentSet.begin()); }
    iterator end() { return iterator(m_fragmentSet.end()); }
    const_iterator begin() const { return const_iterator(m_fragmentSet.begin()); }
    const_iterator end() const { return const_iterator(m_fragmentSet.end()); }

    reverse_iterator rbegin() { return reverse_iterator(m_fragmentSet.rbegin()); }
    reverse_iterator rend() { return reverse_iterator(m_fragmentSet.rend()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(m_fragmentSet.rbegin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(m_fragmentSet.rend()); }

public:
    UserProc *getProc() { return m_myProc; }
    const UserProc *getProc() const { return m_myProc; }

    /// Remove all IRFragments in the CFG
    void clear();

    /// \returns the number of fragments in this CFG.
    int getNumFragments() const { return m_fragmentSet.size(); }

    /// Checks if the fragment is part of this CFG
    bool hasFragment(const IRFragment *frag) const;

    /// Create a new fragment with the given semantics and add it to this CFG.
    /// \returns the newly created fragment.
    IRFragment *createFragment(std::unique_ptr<RTLList> rtls, BasicBlock *bb);

    /// Split the given fragment in two at the given address, if possible.
    /// If the split is successful, returns the new fragment containing the RTLs
    /// after the split address. If the split is unsuccessful, returns the original fragment.
    IRFragment *splitFragment(IRFragment *frag, Address splitAddr);

    /// \returns the entry fragment of the procedure of this CFG
    IRFragment *getEntryFragment() { return m_entryFrag; }
    const IRFragment *getEntryFragment() const { return m_entryFrag; }
    IRFragment *getExitFragment() { return m_exitFrag; }
    const IRFragment *getExitFragment() const { return m_exitFrag; }

    /// Set the entry fragment to \p entryFrag and mark all return fragments as exit fragments.
    void setEntryAndExitFragment(IRFragment *entryFrag);

    /// Completely removes a single fragment from this CFG.
    /// \note \p frag is invalid after this function returns.
    void removeFragment(IRFragment *frag);

    /// \returns the fragment that starts at \p addr
    IRFragment *getFragmentByAddr(Address addr);

    /// Add an edge from \p sourceFrag to \p destFrag.
    void addEdge(IRFragment *sourceFrag, IRFragment *destFrag);

    /// Checks if all out edges are valid.
    /// Also checks that the CFG does not contain interprocedural edges.
    /// By definition, the empty CFG is well-formed.
    bool isWellFormed() const;

    /// \returns the return fragment. Prioritizes Ret type fragments,
    /// but noreturn calls are also considered if there is no return statement.
    IRFragment *findRetFragment();

    // Implicit assignments

    /// Find the existing implicit assign for x (if any)
    SharedStmt findTheImplicitAssign(const SharedConstExp &x) const;

    /// Find exiting implicit assign for parameter p
    SharedStmt findImplicitParamAssign(Parameter *p);

    /// Remove an existing implicit assignment for x
    void removeImplicitAssign(SharedExp x);

    /// Find or create an implicit assign for x
    SharedStmt findOrCreateImplicitAssign(SharedExp x);

    bool isImplicitsDone() const { return m_implicitsDone; }
    void setImplicitsDone() { m_implicitsDone = true; }

public:
    /// print this CFG, mainly for debugging
    void print(OStream &out) const;

    QString toString() const;

private:
    FragmentSet::iterator findFragment(const IRFragment *frag) const;

private:
    UserProc *m_myProc = nullptr;      ///< Procedure to which this CFG belongs.
    FragmentSet m_fragmentSet;         ///< The set hoding all fragments for this proc
    IRFragment *m_entryFrag = nullptr; ///< The CFG entry fragment.
    IRFragment *m_exitFrag  = nullptr; ///< The CFG exit fragment.

    /// Map from expression to implicit assignment. The purpose is to prevent
    /// multiple implicit assignments for the same location.
    ExpStatementMap m_implicitMap;

    /// True when the implicits are done; they can cause problems
    /// (e.g. with ad-hoc global assignment)
    bool m_implicitsDone = false;
};
