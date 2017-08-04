#pragma once

#include "TypeRecovery.h"


class Signature;
class Cfg;
class StatementList;
class Statement;


class DFATypeRecovery : public TypeRecoveryCommon
{
public:
    void recoverFunctionTypes(Function *) override;

    QString name() override { return "data-flow based"; }

    void dfaTypeAnalysis(Function *f);
    bool dfaTypeAnalysis(Signature *sig, Cfg *cfg);
    bool dfaTypeAnalysis(Statement *i);

protected:
    void dumpResults(StatementList& stmts, int iter);

private:
    void dfa_analyze_scaled_array_ref(Statement *s);

    // 3) Check implicit assigns for parameter and global types.
    void dfa_analyze_implict_assigns(Statement *s);
};
