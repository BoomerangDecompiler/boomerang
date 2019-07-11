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
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySymbol.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/type/FuncType.h"
#include "boomerang/util/log/Log.h"


GlobalConstReplacePass::GlobalConstReplacePass()
    : IPass("GlobalConstReplace", PassID::GlobalConstReplace)
{
}


bool GlobalConstReplacePass::execute(UserProc *proc)
{
    StatementList stmts;
    proc->getStatements(stmts);

    const BinaryImage *image      = proc->getProg()->getBinaryFile()->getImage();
    const BinarySymbolTable *syms = proc->getProg()->getBinaryFile()->getSymbols();
    bool changed                  = false;

    for (Statement *st : stmts) {
        Assign *assgn = dynamic_cast<Assign *>(st);

        if (assgn == nullptr) {
            continue;
        }
        else if (!assgn->getRight()->isMemOf()) {
            continue;
        }
        else if (!assgn->getRight()->getSubExp1()->isIntConst()) {
            continue;
        }

        const Address addr      = assgn->getRight()->access<Const, 1>()->getAddr();
        const BinarySymbol *sym = syms->findSymbolByAddress(addr);
        if (sym && sym->isImportedFunction()) {
            LibProc *libProc = proc->getProg()->getOrCreateLibraryProc(sym->getName());
            libProc->setEntryAddress(addr);
            assgn->setRight(Const::get(libProc));
            assgn->setType(FuncType::get(libProc->getSignature()));
            changed = true;
        }
        else if (proc->getProg()->isReadOnly(addr)) {
            switch (assgn->getType()->getSize()) {
            case 8: {
                Byte value = 0;
                if (image->readNative1(addr, value)) {
                    assgn->setRight(Const::get(value));
                    changed = true;
                }
                break;
            }
            case 16: {
                SWord value = 0;
                if (image->readNative2(addr, value)) {
                    assgn->setRight(Const::get(value));
                    changed = true;
                }
                break;
            }
            case 32: {
                DWord value = 0;
                if (image->readNative4(addr, value)) {
                    assgn->setRight(Const::get(value));
                    changed = true;
                }
                break;
            }
            case 64: {
                QWord value = 0;
                if (image->readNative8(addr, value)) {
                    assgn->setRight(Const::get(value));
                    changed = true;
                }
                break;
            }
            case 80: continue; // can't replace float constants just yet
            default: assert(false);
            }
        }
    }

    return changed;
}
