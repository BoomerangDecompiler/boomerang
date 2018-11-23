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


#include "boomerang/ifc/ITypeRecovery.h"

#include <QString>


class TypeRecoveryCommon : public ITypeRecovery
{
public:
    TypeRecoveryCommon(Project *project, const QString &name);

public:
    /// \copydoc ITypeRecovery::getName
    const QString &getName() override;

    /// \copydoc ITypeRecovery::recoverProgramTypes
    virtual void recoverProgramTypes(Prog *prog) override;

private:
    const QString m_name;
};
