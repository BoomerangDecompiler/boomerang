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


#include <memory>


class QByteArray;
class BinaryImage;
class BinarySymbolTable;


class BinaryFile
{
public:
    BinaryFile(const QByteArray& rawData);

public:
    BinaryImage *getImage() { return m_image.get(); }
    const BinaryImage *getImage() const { return m_image.get(); }

    BinarySymbolTable *getSymbols() { return m_symbols.get(); }
    const BinarySymbolTable *getSymbols() const { return m_symbols.get(); }

private:
    std::unique_ptr<BinaryImage> m_image;
    std::unique_ptr<BinarySymbolTable> m_symbols;
};
