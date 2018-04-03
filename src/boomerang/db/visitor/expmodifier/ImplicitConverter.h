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


#include "boomerang/db/visitor/expmodifier/ExpModifier.h"


class Cfg;


/**
 * Convert any exp{-} (with null definition) so that the definition points instead to an implicit assignment (exp{0})
 * Note it is important to process refs in a depth first manner, so that e.g. m[sp{-}-8]{-} -> m[sp{0}-8]{-} first, so
 * that there is never an implicit definition for m[sp{-}-8], only ever for m[sp{0}-8]
 */
class ImplicitConverter : public ExpModifier
{
public:
    ImplicitConverter(Cfg *cfg);
    virtual ~ImplicitConverter() = default;

public:
    /// \copydoc ExpModifier::postModify
    // This is in the POST visit function, because it's important to process any child expressions first.
    // Otherwise, for m[r28{0} - 12]{0}, you could be adding an implicit assignment with a nullptr definition for r28.
    SharedExp postModify(const std::shared_ptr<RefExp>& exp) override;

private:
    Cfg *m_cfg;
};
