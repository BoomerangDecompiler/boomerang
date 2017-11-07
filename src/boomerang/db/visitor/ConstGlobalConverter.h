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

class Prog;


/**
 * Constant global converter.
 * Example: m[m[r24{16} + m[0x8048d60]{-}]{-}]{-} -> m[m[r24{16} + 32]{-}]{-}
 * Allows some complex variations to be matched to standard indirect call forms
 */
class ConstGlobalConverter : public ExpModifier
{
public:
    ConstGlobalConverter(Prog *pg);

    /// \copydoc ExpModifier::preVisit
    SharedExp preVisit(const std::shared_ptr<RefExp>& exp, bool& visitChildren) override;

private:
    Prog *m_prog; ///< Pointer to the Prog object, for reading memory
};
