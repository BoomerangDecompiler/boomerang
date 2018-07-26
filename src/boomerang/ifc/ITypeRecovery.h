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


#include <QString>

class Function;
class Prog;


/**
 * Base class for type recovery engines.
 */
class ITypeRecovery
{
public:
    ITypeRecovery() = default;

    ITypeRecovery(const ITypeRecovery& other) = delete;
    ITypeRecovery(ITypeRecovery&& other) = default;

    virtual ~ITypeRecovery() = default;

    ITypeRecovery& operator=(const ITypeRecovery& other) = delete;
    ITypeRecovery& operator=(ITypeRecovery&& other) = default;

public:
    /// \returns A descriptive name of this type recovery engine.
    virtual QString getName() = 0;

    /// Recover program types for a single function \p function
    virtual void recoverFunctionTypes(Function *function) = 0;

    /// Recover program types for the whole program
    virtual void recoverProgramTypes(Prog *prog)          = 0;
};
