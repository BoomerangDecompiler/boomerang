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


#include "boomerang/db/visitor/ExpVisitor.h"

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
    UsedLocalFinder(LocationSet& _used, UserProc *_proc);
    ~UsedLocalFinder() override = default;

    LocationSet *getLocSet() { return used; }
    bool wasAllFound() { return all; }

    virtual bool visit(const std::shared_ptr<Location>& e, bool& override) override;
    virtual bool visit(const std::shared_ptr<TypedExp>& e, bool& override) override;
    virtual bool visit(const std::shared_ptr<Terminal>& e) override;

private:
    LocationSet *used; // Set of used locals' names
    UserProc *proc;    // Enclosing proc
    bool all;          // True if see opDefineAll
};

