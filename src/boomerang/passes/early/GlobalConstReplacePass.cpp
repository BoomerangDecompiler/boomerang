#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "GlobalConstReplacePass.h"


#include "boomerang/db/Prog.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/binary/BinaryImage.h"


GlobalConstReplacePass::GlobalConstReplacePass()
    : IPass("GlobalConstReplace", PassID::GlobalConstReplace)
{
}


bool GlobalConstReplacePass::execute(UserProc *proc)
{
    StatementList stmts;
    proc->getStatements(stmts);

    const BinaryImage *image = proc->getProg()->getBinaryFile()->getImage();

    for (Statement *st : stmts) {
        Assign *assgn = dynamic_cast<Assign *>(st);

        if (assgn == nullptr) {
            continue;
        }

        if (!assgn->getRight()->isMemOf()) {
            continue;
        }

        if (!assgn->getRight()->getSubExp1()->isIntConst()) {
            continue;
        }

        Address addr = assgn->getRight()->access<Const, 1>()->getAddr();
        LOG_VERBOSE("Assign %1");

        if (proc->getProg()->isReadOnly(addr)) {
            LOG_VERBOSE("is readonly");
            int val = 0;

            switch (assgn->getType()->getSize())
            {
            case  8: val = image->readNative1(addr); break;
            case 16: val = image->readNative2(addr); break;
            case 32: val = image->readNative4(addr); break;
            default: assert(false);
            }

            assgn->setRight(Const::get(val));
        }
    }
    return true;
}
