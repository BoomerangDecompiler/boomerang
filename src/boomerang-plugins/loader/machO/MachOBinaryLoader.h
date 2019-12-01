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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/ifc/IFileLoader.h"

#include <string>
#include <vector>


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

    virtual std::map<QString, ObjcModule> &getObjcModules() = 0;
};


/// Loader for Mach-O binary files.
class BOOMERANG_PLUGIN_API MachOBinaryLoader : public IFileLoader, public ObjcAccessInterface
{
public:
    MachOBinaryLoader(Project *project);
    ~MachOBinaryLoader() override;

public:
    /// \copydoc IFileLoader::initialize
    void initialize(BinaryFile *filee, BinarySymbolTable *symbols) override;

    /// \copydoc IFileLoader::loadFromMemory
    bool loadFromMemory(QByteArray &data) override;

    /// \copydoc IFileLoader::canLoad
    int canLoad(QIODevice &dev) const override;

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

    std::map<QString, ObjcModule> &getObjcModules() override { return modules; }

protected:
    SWord machORead2(const void *ps) const;
    DWord machORead4(const void *pi) const;

    int32_t BMMH(int32_t x);
    DWord BMMH(DWord x);
    SWord BMMHW(SWord x);

private:
    /// Find names for jumps to IATs
    void findJumps(Address curr);

private:
    char *base           = nullptr; ///< Beginning of the loaded image
    Address entrypoint   = Address::INVALID;
    Address loaded_addr  = Address::INVALID;
    unsigned loaded_size = 0;
    Machine machine      = Machine::INVALID;
    bool swap_bytes      = false;

    std::map<QString, ObjcModule> modules;
    std::vector<struct section> sections;
    BinaryImage *Image         = nullptr;
    BinarySymbolTable *Symbols = nullptr;
};
