#include "TypeRecovery.h"

#include "boomerang/util/Log.h"

#include "boomerang/db/Proc.h"
#include "boomerang/db/Prog.h"


void TypeRecoveryCommon::recoverProgramTypes(Prog *v)
{
    if (VERBOSE || DEBUG_TA) {
        LOG << "=== start " << name() << " type analysis ===\n";
    }

    // FIXME: This needs to be done in bottom-up order of the call-tree first,
    // repeating until no changes for cycles in the call graph
    for (Module *module : v->getModuleList()) {
        for (Function *pp : *module) {
            UserProc *proc = dynamic_cast<UserProc *>(pp);

            if ((nullptr == proc) || !proc->isDecoded()) {
                continue;
            }

            // FIXME: this just does local TA again. Need to resolve types for all parameter/arguments,
            // and return/results! This will require a "repeat until no change" loop
            LOG_STREAM() << "global type analysis for " << proc->getName() << "\n";
            recoverFunctionTypes(pp);
        }
    }

    if (VERBOSE || DEBUG_TA) {
        LOG << "=== end type analysis ===\n";
    }
}
