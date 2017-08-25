#pragma once

/*
 * Copyright (C) 2000, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file MachOBinaryLoader.h
 * \brief This file contains the definition of the class MachOBinaryLoader.
 */

#include "boomerang/loader/IFileLoader.h"

#include <string>
#include <vector>


/**
 * This file contains the definition of the MachOBinaryLoader class,
 * This is my bare bones implementation of a Mac OS-X binary loader.
 */

// Given a little endian value x, load its value assuming big endian order
// Note: must be able to take address of x
// Note: Unlike the LH macro in BinaryFile.h, the parameter is not a pointer
#define _BMMH(x)                                                                                              \
    ((unsigned)((Byte *)(&x))[3] + ((unsigned)((Byte *)(&x))[2] << 8) + ((unsigned)((Byte *)(&x))[1] << 16) + \
     ((unsigned)((Byte *)(&x))[0] << 24))
// With this one, x IS a pounsigneder
#define _BMMH2(x)                                                                                           \
    ((unsigned)((Byte *)(x))[3] + ((unsigned)((Byte *)(x))[2] << 8) + ((unsigned)((Byte *)(x))[1] << 16) + \
     ((unsigned)((Byte *)(x))[0] << 24))

#define _BMMHW(x)    (((unsigned)((Byte *)(&x))[1]) + ((unsigned)((Byte *)(&x))[0] << 8))

struct mach_header;



// Objective-C stuff
class ObjcIvar
{
public:
    QString name, type;
    unsigned offset;
};


class ObjcMethod
{
public:
    QString name, types;
       Address addr;
};


class ObjcClass
{
public:
    QString name;
    std::map<QString, ObjcIvar> ivars;
    std::map<QString, ObjcMethod> methods;
};


class ObjcModule
{
public:
    QString name;
    std::map<QString, ObjcClass> classes;
};


class ObjcAccessInterface
{
public:
    virtual ~ObjcAccessInterface() {}

    virtual std::map<QString, ObjcModule>& getObjcModules() = 0;
};


class MachOBinaryLoader : public IFileLoader, public ObjcAccessInterface
{
public:
    MachOBinaryLoader();                 // Default constructor
    virtual ~MachOBinaryLoader();        // Destructor

    /// \copydoc IFileLoader::initialize
    void initialize(IBinaryImage *image, IBinarySymbolTable *symbols) override;

    /// \copydoc IFileLoader::loadFromMemory
    bool loadFromMemory(QByteArray& data) override;

    /// \copydoc IFileLoader::canLoad
    int canLoad(QIODevice& dev) const override;

    /// \copydoc IFileLoader::unload
    void unload() override;

    /// \copydoc IFileLoader::close
    void close() override;

    /// \copydoc IFileLoader::getFormat
    LoadFmt getFormat() const override;

    /// \copydoc IFileLoader::getMachine
    Machine getMachine() const override;

    /// \copydoc IFileLoader::getMainEntryPoint
    Address getMainEntryPoint() override;

    /// \copydoc IFileLoader::getEntryPoint
    Address getEntryPoint() override;

    DWord getDelta();

    bool isLibrary() const;

    std::map<QString, ObjcModule>& getObjcModules() override  { return modules; }

protected:
    int machORead2(short *ps) const; // Read 2 bytes from native addr
    int machORead4(int *pi) const;   // Read 4 bytes from native addr

    // void *            BMMH(void *x);
    // char *            BMMH(char *x);
    //    const char *        BMMH(const char *x);
    // unsigned int        BMMH(long int & x);
    int32_t BMMH(int32_t x);
    uint32_t BMMH(uint32_t x);
    unsigned short BMMHW(unsigned short x);

private:
    /// Find names for jumps to IATs
    void findJumps(Address curr);

private:
    char *base;           ///< Beginning of the loaded image
    Address entrypoint;
    Address loaded_addr;
    unsigned loaded_size;
    Machine machine;
    bool swap_bytes;

    std::map<QString, ObjcModule> modules;
    std::vector<struct section> sections;
    IBinaryImage *Image;
    IBinarySymbolTable *Symbols;
};
