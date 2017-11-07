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

class Statement;


class ExpSubscripter : public ExpModifier
{
public:
    ExpSubscripter(const SharedExp& s, Statement *d);

    SharedExp preVisit(const std::shared_ptr<Location>& e, bool& recur) override;
    SharedExp preVisit(const std::shared_ptr<Binary>& e, bool& recur) override;
    SharedExp preVisit(const std::shared_ptr<Terminal>& e) override;
    SharedExp preVisit(const std::shared_ptr<RefExp>& e, bool& recur) override;

private:
    SharedExp m_search;
    Statement *m_def;
};



