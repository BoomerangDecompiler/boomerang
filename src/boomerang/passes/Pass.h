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
    virtual bool runOnFunction(Function& function) = 0;
};
