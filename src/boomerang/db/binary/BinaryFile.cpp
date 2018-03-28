#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BinaryFile.h"


#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/loader/IFileLoader.h"


BinaryFile::BinaryFile(const QByteArray& rawData, IFileLoader *loader)
    : m_image(new BinaryImage(rawData))
    , m_symbols(new BinarySymbolTable())
    , m_loader(loader)
{
}


BinaryImage *BinaryFile::getImage()
{
    return m_image.get();
}


const BinaryImage *BinaryFile::getImage() const
{
    return m_image.get();
}


BinarySymbolTable *BinaryFile::getSymbols()
{
    return m_symbols.get();
}


const BinarySymbolTable *BinaryFile::getSymbols() const
{
    return m_symbols.get();
}


LoadFmt BinaryFile::getFormat() const
{
    return m_loader->getFormat();
}


Machine BinaryFile::getMachine() const
{
    return m_loader->getMachine();
}


Address BinaryFile::getEntryPoint() const
{
    return m_loader->getEntryPoint();
}


Address BinaryFile::getMainEntryPoint() const
{
    return m_loader->getMainEntryPoint();
}


bool BinaryFile::isRelocationAt(Address addr) const
{
    return m_loader->isRelocationAt(addr);
}


Address BinaryFile::getJumpTarget(Address addr) const
{
    return m_loader->getJumpTarget(addr);
}


bool BinaryFile::hasDebugInfo() const
{
    return false;
}
