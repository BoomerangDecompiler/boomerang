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


#include "boomerang/visitor/expmodifier/SimpExpModifier.h"


class CallStatement;


/**
 * Localiser. Subscript a location with the definitions that reach the call,
 * or with {-} if none
 */
class Localiser : public SimpExpModifier
{
public:
    Localiser(CallStatement *call);
    virtual ~Localiser() = default;

public:
    /// \copydoc SimpExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<RefExp>& exp, bool& visitChildren) override;

    /// \copydoc SimpExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<Location>& exp, bool& visitChildren) override;

    /// \copydoc SimpExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<Location>& exp) override;

    /// \copydoc SimpExpModifier::postModify
    /// We want to be able to localise a few terminals, in particular <all>
    SharedExp modify(const std::shared_ptr<Terminal>& exp) override;

private:
    CallStatement *m_call; ///< The call to localise to
};
