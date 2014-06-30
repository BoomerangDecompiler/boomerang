#ifndef IBOOMERANG_H
#define IBOOMERANG_H
/***************************************************************************/ /**
  * \file       IBoomerang.h
  *   Interface for the core system, accessible from all plugins
  * TODO: consider the way it's done in qtcreator's plugins system
  ******************************************************************************/
class IBinaryImage;
class IBinarySymbolTable;
class IBoomerang {
public:
    virtual IBinaryImage *getImage() = 0;
    virtual IBinarySymbolTable *getSymbols() = 0;
};

#endif // IBOOMERANG_H
