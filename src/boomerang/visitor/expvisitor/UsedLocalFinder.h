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


#include "boomerang/visitor/expvisitor/ExpVisitor.h"


class LocationSet;
class UserProc;


/**
 * This class differs from the above in these ways:
 *  1) it counts locals implicitly referred to with (cast to pointer)(sp-K)
 *  2) it does not recurse inside the memof (thus finding the stack pointer as a local)
 *  3) only used after fromSSA, so no RefExps to visit
 */
class UsedLocalFinder : public ExpVisitor
{
public:
    UsedLocalFinder(LocationSet &_used, UserProc *_proc);
    virtual ~UsedLocalFinder() = default;

public:
    LocationSet *getLocSet() { return m_used; }
    bool wasAllFound() { return all; }

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Location> &exp, bool &visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<TypedExp> &exp, bool &visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    bool visit(const std::shared_ptr<Terminal> &exp) override;

private:
    LocationSet *m_used; // Set of used locals' names
    UserProc *m_proc;    // Enclosing proc
    bool all;            // True if see opDefineAll
};
