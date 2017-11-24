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


class IBinaryImage;
class IBinarySymbolTable;
class IProject;


/**
 * Interface for the core system, accessible from all plugins
 * \todo consider the way it's done in qtcreator's plugins system
 */
class IBoomerang
{
public:
    virtual ~IBoomerang() = default;

    virtual IBinaryImage *getImage()         = 0;
    virtual IBinarySymbolTable *getSymbols() = 0;
    virtual IProject *getOrCreateProject()   = 0;
};
