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


#include "boomerang/ssl/exp/Exp.h"
#include "boomerang/util/ExpSet.h"


class RefExp;
class Statement;


/**
 * For various purposes, we need sets of locations (registers or memory).
 * \note we cannot subclass directly from ExpSet<Location>, because
 * the expressions might be subscripted (->RefExp).
 */
class BOOMERANG_API LocationSet : public ExpSet<Exp, lessExpStar>
{
public:
    LocationSet() = default;
    LocationSet(const std::initializer_list<SharedExp> &exps);
    LocationSet(const LocationSet &other);
    LocationSet(LocationSet &&other) = default;

    ~LocationSet() = default;

    LocationSet &operator=(const LocationSet &other);
    LocationSet &operator=(LocationSet &&other) = default;

public:
    /// Given an unsubscripted location \p e, return true if e{-} or e{0} exists in the set
    bool containsImplicit(SharedExp e) const;

    /**
     * Given a not subscripted location \p e, return the subscripted location matching \p e.
     * Example: Given \p e == r32, return r32{-}.
     * Returns nullptr if not found.
     * \note This set is assumed to be of subscripted locations (e.g. a Collector).
     */
    SharedExp findNS(SharedExp e);

    /// Find a location with a different def, but same expression.
    /// For example, pass r28{10}, return true if r28{20} is in the set.
    /// If return true, \p differentRef points to the first different ref
    bool findDifferentRef(const std::shared_ptr<RefExp> &ref, SharedExp &differentRef);

    /// Add a subscript (to definition \p def) to each element.
    /// Existing exps are not re-subscripted.
    void addSubscript(Statement *def);

    QString toString() const; ///< Print to string for debugging
};
