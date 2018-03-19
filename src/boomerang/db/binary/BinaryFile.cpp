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


BinaryFile::BinaryFile(const QByteArray& rawData)
    : m_image(new BinaryImage(rawData))
    , m_symbols(new BinarySymbolTable())
{
}
