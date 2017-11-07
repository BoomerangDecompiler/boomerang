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

using SharedType = std::shared_ptr<class Type>;


/**
 *
 */
class ExpCastInserter : public ExpModifier
{
public:
    ExpCastInserter() = default;

    static void checkMemofType(const SharedExp& memof, SharedType memofType);

    SharedExp postVisit(const std::shared_ptr<RefExp>& e) override;
    SharedExp postVisit(const std::shared_ptr<Binary>& e) override;
    SharedExp postVisit(const std::shared_ptr<Const>& e) override;

    SharedExp preVisit(const std::shared_ptr<TypedExp>& e, bool& recur) override; // Don't consider if already cast
};
