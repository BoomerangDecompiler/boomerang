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


#include "TypeRecovery.h"

#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/ssl/statements/Statement.h"

#include <list>


class ProcCFG;
class Signature;
class StatementList;
class UserProc;
class Const;


/**
 * Data-flow based type recovery.
 *
 * \internal
 * This is the core of the data-flow-based type analysis algorithm: implementing the meet operator.
 * In classic lattice-based terms, the TOP type is void; there is no BOTTOM type since we handle
 * overconstraints with unions. Consider various pieces of knowledge about the types. There could
 * be:
 *   a) void: no information. Void meet x = x.
 *   b) size only: find a size large enough to contain the two types.
 *   c) broad type only, e.g. floating point
 *   d) signedness, no size
 *   e) size, no signedness
 *   f) broad type, size, and (for integer broad type), signedness
 */
class BOOMERANG_PLUGIN_API DFATypeRecovery : public TypeRecoveryCommon
{
public:
    DFATypeRecovery(Project *project);
    virtual ~DFATypeRecovery() = default;

public:
    /// \copydoc ITypeRecovery::recoverFunctionTypes
    void recoverFunctionTypes(Function *function) override;

private:
    void dfaTypeAnalysis(UserProc *proc);
    bool dfaTypeAnalysis(Signature *signature, ProcCFG *cfg);
    //     bool dfaTypeAnalysis(const SharedStmt &stmt);

    void printResults(StatementList &stmts, int iter);

    /// Replace array references of the form m[idx*K1 + K2]
    /// in \p s. Create global array variables as needed.
    void replaceArrayIndices(const SharedStmt &s);

    // 3) Check implicit assigns for parameter and global types.
    void dfa_analyze_implict_assigns(const SharedStmt &s);

    /**
     * Trim parameters to procedure calls with ellipsis (...).
     * Also add types for ellipsis parameters, if any
     * \returns true if any signature types so added.
     */
    bool doEllipsisProcessing(UserProc *proc);

    void findConstantsInStmt(const SharedStmt &stmt, std::list<std::shared_ptr<Const>> &constants);
};
