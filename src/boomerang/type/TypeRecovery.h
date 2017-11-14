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
    virtual ~ITypeRecovery() = default;

    /// \returns A descriptive name of this type recovery engine.
    virtual QString getName() = 0;

    /// Recover program types for a single function \p function
    virtual void recoverFunctionTypes(Function *function) = 0;

    /// Recover program types for the whole program
    virtual void recoverProgramTypes(Prog *prog)          = 0;
};


class TypeRecoveryCommon : public ITypeRecovery
{
public:
    /// \copydoc ITypeRecovery::recoverProgramTypes
    virtual void recoverProgramTypes(Prog *prog) override;
};
