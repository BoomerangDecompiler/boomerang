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


class IPass
{
public:
    IPass() = default;
    virtual ~IPass() = default;
};


class FunctionPass : public IPass
{
public:
    virtual ~FunctionPass() override = default;

    virtual bool runOnFunction(Function& function) = 0;
};
