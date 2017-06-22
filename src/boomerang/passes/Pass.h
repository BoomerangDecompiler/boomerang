#pragma once

class Function;

class Pass
{
public:
	Pass();
};


class FunctionPass : public Pass
{
	virtual bool runOnFunction(Function& F) = 0;
};
