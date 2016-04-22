#pragma once
#include <QString>
class Function;
class Prog;

struct ITypeRecovery {
    virtual ~ITypeRecovery() {}
    virtual void recoverFunctionTypes(Function *) = 0;
    virtual void recoverProgramTypes(Prog *) = 0;
    virtual QString name() = 0;
};


struct TypeRecoveryCommon : public ITypeRecovery {
    virtual void recoverProgramTypes(Prog *v) override;
};
