#ifndef PASS_H
#define PASS_H
class Function;
class Pass
{
public:
    Pass();
};
class FunctionPass : public Pass {
    virtual bool runOnFunction(Function &F)=0;
};
#endif // PASS_H
