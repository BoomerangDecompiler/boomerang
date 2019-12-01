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


#include "boomerang/visitor/expmodifier/ExpModifier.h"


class UserProc;


/**
 * Transform an exp by applying mappings to the subscripts.
 * This used to be done by many Exp::fromSSAForm() functions.
 * Note that mappings have to be done depth first,
 * so e.g. m[r28{0}-8]{22} -> m[esp-8]{22} first,
 * otherwise there wil be a second implicit definition
 * for m[esp{0}-8] (original should be b[esp+8] by now)
 */
class ExpSSAXformer : public ExpModifier
{
public:
    ExpSSAXformer(UserProc *p);
    virtual ~ExpSSAXformer() = default;

public:
    UserProc *getProc() { return m_proc; }

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<RefExp> &exp) override;

private:
    UserProc *m_proc;
};
