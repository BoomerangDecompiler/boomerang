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


#include "boomerang/db/visitor/ExpModifier.h"


class UserProc;
class Prog;
class Signature;


/**
 * This class is an ExpModifier because although most of the time it merely maps expressions to locals, in one case,
 * where sp-K is found, we replace it with a[m[sp-K]] so the back end emits it as &localX.
 * FIXME: this is probably no longer necessary, since the back end no longer maps anything!
 */
class DfaLocalMapper : public ExpModifier
{
public:
    /**
     * Map expressions to locals, using the (so far DFA based) type analysis information
     * Basically, descend types, and when you get to m[...] compare with the local high level pattern;
     * when at a sum or difference, check for the address of locals high level pattern that is a pointer
     */
    DfaLocalMapper(UserProc *proc);
    virtual ~DfaLocalMapper() = default;

public:
    /// \copydoc ExpModifier::preVisit
    /// To process m[X]
    SharedExp preVisit(const std::shared_ptr<Location>& exp, bool& visitChildren) override;

    /// \copydoc ExpModifier::preVisit
    /// To look for sp -+ K
    SharedExp preVisit(const std::shared_ptr<Binary>& exp, bool& visitChildren) override;

    /// \copydoc ExpModifier::preVisit
    /// To prevent processing TypedExps more than once
    SharedExp preVisit(const std::shared_ptr<TypedExp>& exp, bool& visitChildren) override;

public:
    bool change = false; // True if changed this statement

private:
    UserProc *m_proc = nullptr;
    Prog *m_prog = nullptr;
    std::shared_ptr<Signature> m_sig;      ///< Look up once (from proc) for speed

    /// Common processing for the two main cases (visiting a Location or a Binary)
    bool processExp(const SharedExp& e);
};

