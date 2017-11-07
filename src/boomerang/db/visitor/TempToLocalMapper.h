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

class UserProc;


/**
 *
 */
class TempToLocalMapper : public ExpVisitor
{
public:
    TempToLocalMapper(UserProc *p);

    bool visit(const std::shared_ptr<Location>& e, bool& override) override;

private:
    UserProc *proc; // Proc object for storing the symbols
};
