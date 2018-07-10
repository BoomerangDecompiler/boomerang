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


class UserProc;
class QString;


class UseGraphWriter
{
public:
    void writeUseGraph(const UserProc *proc, const QString& fileName);
};
