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
#include "boomerang/ifc/IFileLoader.h"


BinaryFile::BinaryFile(const QByteArray &rawData, IFileLoader *loader)
    : m_image(new BinaryImage(rawData))
    , m_symbols(new BinarySymbolTable())
    , m_loader(loader)
{
}


BinaryFile::~BinaryFile()
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
    return m_loader ? m_loader->getFormat() : LoadFmt::INVALID;
}


Machine BinaryFile::getMachine() const
{
    return m_loader ? m_loader->getMachine() : Machine::INVALID;
}


Address BinaryFile::getEntryPoint() const
{
    return m_loader ? m_loader->getEntryPoint() : Address::INVALID;
}


Address BinaryFile::getMainEntryPoint() const
{
    return m_loader ? m_loader->getMainEntryPoint() : Address::INVALID;
}


bool BinaryFile::isRelocationAt(Address addr) const
{
    return m_loader ? m_loader->isRelocationAt(addr) : false;
}


Address BinaryFile::getJumpTarget(Address addr) const
{
    return m_loader ? m_loader->getJumpTarget(addr) : Address::INVALID;
}


bool BinaryFile::hasDebugInfo() const
{
    return false;
}


int BinaryFile::getBitness() const
{
    return m_bitness;
}


void BinaryFile::setBitness(int bitness)
{
    m_bitness = bitness;
}
