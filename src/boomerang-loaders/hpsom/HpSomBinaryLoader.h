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


#include "boomerang/db/binary/IBinarySymbols.h"
#include "boomerang/loader/IFileLoader.h"

#include <set>

struct import_entry
{
    int   name;
    short reserved2;
    Byte  type;
    Byte  reserved1;
};

struct export_entry
{
    int   next;
    int   name;
    int   value;
    int   size; // Also misc_info
    Byte  type;
    char  reserved1;
    short module_index;
};

struct space_dictionary_record
{
    unsigned name;
    unsigned flags;
    int      space_number;
    int      subspace_index;
    unsigned subspace_quantity;
    int      loader_fix_index;
    unsigned loader_fix_quantity;
    int      init_pointer_index;
    unsigned init_pointer_quantity;
};

struct subspace_dictionary_record
{
    int      space_index;
    unsigned flags;
    int      file_loc_init_value;
    unsigned initialization_length;
    unsigned subspace_start;
    unsigned subspace_length;
    unsigned alignment;
    unsigned name;
    int      fixup_request_index;
    int      fixup_request_quantity;
};

struct plt_record
{
    uint32_t value;    // Address in the library
    uint32_t r19value; // r19 value needed
};

struct symElem
{
    const char *name; // Simple symbol table entry
    Address    value;
};


/**
 * This class provides is responsible for decoding PA/RISC SOM executable files.
 */
class HpSomBinaryLoader : public IFileLoader
{
public:
    HpSomBinaryLoader();    // Constructor
    virtual ~HpSomBinaryLoader();

public:
    /// \copydoc IFileLoader::initialize
    void initialize(BinaryImage *image, IBinarySymbolTable *symbols) override;

    /// \copydoc IFileLoader::canLoad
    int canLoad(QIODevice& dev) const override;

    /// \copydoc IFileLoader::loadFromMemory
    bool loadFromMemory(QByteArray& data) override;

    /// \copydoc IFileLoader::unload
    void unload() override;

    /// \copydoc IFileLoader::close
    void close() override;

    /// \copydoc IFileLoader::getMainEntryPoint
    Address getMainEntryPoint() override;

    /// \copydoc IFileLoader::getEntryPoint
    Address getEntryPoint() override;


    /// \copydoc IFileLoader::getFormat
    LoadFmt getFormat() const override;

    /// \copydoc IFileLoader::getMachine
    Machine getMachine() const override;

    bool isLibrary() const;

private:
    /// Specific to BinaryFile objects that implement a "global pointer"
    /// Gets a pair of unsigned integers representing the address of %agp (first)
    /// and the value for GLOBALOFFSET (unused for pa-risc)
    std::pair<Address, unsigned> getGlobalPointerInfo();

    /**
     * Get map containing the addresses and symbolic names of global data items
     * (if any) which are shared with dynamically linked libraries.
     * Example: __iob (basis for stdout). The Address is the (native)
     * address of a pointer to the real dynamic data object.
     *
     * \note        The caller should delete the returned map.
     * \returns     Pointer to a new map with the info
     */
    std::map<Address, const char *> *getDynamicGlobalMap();

    /// Private method to get the start and length of a given subspace
    std::pair<Address, int> getSubspaceInfo(const char *ssname);

    Byte *m_loadedImage;            ///< Points to loaded image
    IBinarySymbolTable *m_symbols;  ///< Symbol table object
    std::set<Address> m_imports;    ///< Set of imported proc addr's
    BinaryImage *m_image;

public:
    void processSymbols();
};
