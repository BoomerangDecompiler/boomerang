#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpArithSimplifier.h"

#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"

#include <numeric>


SharedExp ExpArithSimplifier::preModify(const std::shared_ptr<Unary> &exp, bool &visitChildren)
{
    visitChildren = exp->isAddrOf();
    return exp->shared_from_this();
}


SharedExp ExpArithSimplifier::postModify(const std::shared_ptr<Binary> &exp)
{
    if (exp->getOper() != opPlus && exp->getOper() != opMinus) {
        return exp->shared_from_this();
    }

    // Partition this expression into positive non-integer terms, negative
    // non-integer terms and integer terms.
    std::list<SharedExp> positives;
    std::list<SharedExp> negatives;
    std::vector<int> integers;
    exp->partitionTerms(positives, negatives, integers, false);

    // Now reduce these lists by cancelling pairs
    cancelDuplicates(positives, negatives);

    // Summarise the set of integers to a single number.
    int sum = std::accumulate(integers.begin(), integers.end(), 0);

    // Now put all these elements back together and return the result
    if (positives.empty()) {
        if (negatives.empty()) {
            return Const::get(sum);
        }
        else {
            // No positives, some negatives. sum - Acc
            return Binary::get(opMinus, Const::get(sum), Exp::accumulate(negatives));
        }
    }

    if (negatives.empty()) {
        // Positives + sum
        if (sum == 0) {
            // Just positives
            return Exp::accumulate(positives);
        }
        else {
            OPER _op = opPlus;

            if (sum < 0) {
                _op = opMinus;
                sum = -sum;
            }

            return Binary::get(_op, Exp::accumulate(positives), Const::get(Address(sum)));
        }
    }

    // Some positives, some negatives
    if (sum == 0) {
        // positives - negatives
        return Binary::get(opMinus, Exp::accumulate(positives), Exp::accumulate(negatives));
    }

    // General case: some positives, some negatives, a sum
    OPER _op = opPlus;

    if (sum < 0) {
        _op = opMinus; // Return (pos - negs) - sum
        sum = -sum;
    }

    return Binary::get(_op,
                       Binary::get(opMinus, Exp::accumulate(positives), Exp::accumulate(negatives)),
                       Const::get(sum));
}

void ExpArithSimplifier::cancelDuplicates(std::list<SharedExp> &left, std::list<SharedExp> &right)
{
    // Note: can't improve this algorithm using multisets, since can't instantiate multisets of type
    // Exp (only Exp*). The Exp* in the multisets would be sorted by address, not by value of the
    // expression. So they would be unsorted, same as lists!
    auto itLeft = left.begin();

    while (itLeft != left.end()) {
        auto itRight    = right.begin();
        bool removedOne = false;

        while (itRight != right.end()) {
            if (**itLeft == **itRight) {
                removedOne = true;
                itLeft     = left.erase(itLeft);
                itRight    = right.erase(itRight);
                break;
            }
            else {
                itRight++;
            }
        }

        if (!removedOne) {
            itLeft++;
        }
    }
}
