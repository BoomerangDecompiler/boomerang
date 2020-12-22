#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ST20BinaryLoader.h"

#include "boomerang/core/plugin/Plugin.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/util/ByteUtil.h"
#include "boomerang/util/log/Log.h"

#include <QFile>

#include <stdexcept>


#define ROM_HIGH Address(0x80000000)


ST20BinaryLoader::ST20BinaryLoader(Project *project)
    : IFileLoader(project)
    , m_image(nullptr)
    , m_binaryImage(nullptr)
    , m_symbols(nullptr)
{
}


ST20BinaryLoader::~ST20BinaryLoader()
{
}


void ST20BinaryLoader::initialize(BinaryFile *file, BinarySymbolTable *symbols)
{
    unload();
    m_binaryImage = file->getImage();
    m_symbols     = symbols;
}


int ST20BinaryLoader::canLoad(QIODevice &fl) const
{
    unsigned char buf[8 + sizeof(SWord)];
    if (sizeof(buf) != fl.read(reinterpret_cast<char *>(buf), sizeof(buf))) {
        return 0;
    }

    return Util::testMagic(buf, { 'e', 'v', 'i', 'l', 'e', 'y', 'e', 's' }) ? 8 : 0;
}


bool ST20BinaryLoader::loadFromMemory(QByteArray &arr)
{
    const DWord fileSize = arr.size();

    if (fileSize < 8 + 2) { // sizeof(magic) + 2 bytes for the negative jump
        return false;
    }

    m_image = new char[fileSize];
    memcpy(m_image, arr.constData(), fileSize);

    Address codeStart      = ROM_HIGH - fileSize;
    BinarySection *section = m_binaryImage->createSection("$CODE", codeStart, ROM_HIGH);
    section->setCode(true);
    section->setHostAddr(HostAddress(m_image));

    return true;
}


void ST20BinaryLoader::unload()
{
    if (m_image) {
        delete[] m_image;
        m_image = nullptr;
    }
}


void ST20BinaryLoader::close()
{
    unload();
}


LoadFmt ST20BinaryLoader::getFormat() const
{
    return LoadFmt::ST20;
}


Machine ST20BinaryLoader::getMachine() const
{
    return Machine::ST20;
}


Address ST20BinaryLoader::getMainEntryPoint()
{
    return Address::INVALID;
}


Address ST20BinaryLoader::getEntryPoint()
{
    return ROM_HIGH - 2; // 0x7FFFFFFE
}


Address ST20BinaryLoader::getJumpTarget(Address /*addr*/) const
{
    return Address::INVALID;
}


BOOMERANG_DEFINE_PLUGIN(PluginType::FileLoader, ST20BinaryLoader,
                        "ST20 .bin loader plugin [experimental]", BOOMERANG_VERSION,
                        "Boomerang developers")
