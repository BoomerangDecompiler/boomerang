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


#include "boomerang/db/visitor/SimpExpModifier.h"


class CallStatement;


/**
 * Localiser. Subscript a location with the definitions that reach the call,
 * or with {-} if none
 */
class Localiser : public SimpExpModifier
{
public:
    Localiser(CallStatement *c);

    SharedExp preVisit(const std::shared_ptr<RefExp>& e, bool& recur) override;
    SharedExp preVisit(const std::shared_ptr<Location>& e, bool& recur) override;
    SharedExp postVisit(const std::shared_ptr<Location>& e) override;

    // Want to be able to localise a few terminals, in particular <all>
    SharedExp postVisit(const std::shared_ptr<Terminal>& e) override;

private:
    CallStatement *call; // The call to localise to
};
