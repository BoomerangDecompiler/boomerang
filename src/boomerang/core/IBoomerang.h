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
    virtual ~IBoomerang() {}
    virtual IBinaryImage *getImage()         = 0;
    virtual IBinarySymbolTable *getSymbols() = 0;
    virtual IProject *getOrCreateProject()   = 0;
};
