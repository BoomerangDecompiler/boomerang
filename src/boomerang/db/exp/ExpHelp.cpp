#include "ExpHelp.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/db/exp/Exp.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/util/Log.h"


// A helper class for comparing Exp*'s sensibly
bool lessExpStar::operator()(const SharedConstExp& x, const SharedConstExp& y) const
{
    return (*x < *y);  // Compare the actual Exps
}


bool lessTI::operator()(const SharedExp& x, const SharedExp& y) const
{
    return (*x << *y);  // Compare the actual Exps
}


// A helper class for comparing Assignment*'s sensibly
bool lessAssignment::operator()(const Assignment *x, const Assignment *y) const
{
    return (*x->getLeft() < *y->getLeft());     // Compare the LHS expressions
}


// Repeat the above for Assign's; sometimes the compiler doesn't (yet) understand that Assign's are Assignment's
bool lessAssign::operator()(const Assign *x, const Assign *y) const
{
    return (*x->getLeft() < *y->getLeft());     // Compare the LHS expressions
}


void child(const SharedExp& e, int ind)
{
    if (e == nullptr) {
        Util::alignStream(LOG_STREAM_OLD(), ind + 4) << "<nullptr>\n";
        LOG_STREAM_OLD().flush();
        return;
    }

    e->printx(ind + 4);
}
