#pragma once
#include "TypeRecovery.h"
class Signature;
class Cfg;
class StatementList;
class Instruction;
struct DFA_TypeRecovery : public TypeRecoveryCommon
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
	void dfa_analyze_implict_assigns(Instruction *s);
};
