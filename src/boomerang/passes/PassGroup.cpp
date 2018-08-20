#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PassGroup.h"


PassGroup::PassGroup(const QString &name, const std::initializer_list<IPass *> &passes)
    : m_name(name)
    , m_passes(passes)
{}
