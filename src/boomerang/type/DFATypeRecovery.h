#pragma once

#include "TypeRecovery.h"


class Signature;
class Cfg;
class StatementList;
class Instruction;


class DFATypeRecovery : public TypeRecoveryCommon
{
public:
    void recoverFunctionTypes(Function *) override;

    QString name() override { return "data-flow based"; }

    void dfaTypeAnalysis(Function *f);
    bool dfaTypeAnalysis(Signature *sig, Cfg *cfg);
    bool dfaTypeAnalysis(Instruction *i);

protected:
    void dumpResults(StatementList& stmts, int iter);

private:
    void dfa_analyze_scaled_array_ref(Instruction *s);

    // 3) Check implicit assigns for parameter and global types.
    void dfa_analyze_implict_assigns(Instruction *s);
};
