#pragma once

class Function;

class Pass
{
public:
    Pass();
    virtual ~Pass() {}
};


class FunctionPass : public Pass
{
public:
    virtual ~FunctionPass() {}
    virtual bool runOnFunction(Function& F) = 0;
};
