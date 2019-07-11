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


#include "boomerang/util/OStream.h"

#include <QFile>
#include <QStringList>

#include <map>


class Module;


class CodeWriter
{
    struct WriteDest
    {
    public:
        WriteDest(const QString &outFileName);
        WriteDest(const WriteDest &) = delete;
        WriteDest(WriteDest &&)      = delete;

        ~WriteDest();

    public:
        WriteDest &operator=(const WriteDest &) = delete;
        WriteDest &operator=(WriteDest &&) = delete;

    public:
        template<typename T>
        OStream &operator<<(T val)
        {
            m_os << val;
            m_os.flush();
            return m_os;
        }

    private:
        QFile m_outFile;
        OStream m_os;
    };

    typedef std::map<const Module *, WriteDest> WriteDestMap;

public:
    CodeWriter();
    CodeWriter(const CodeWriter &) = delete;
    CodeWriter(CodeWriter &&)      = default;

    ~CodeWriter() = default;

    CodeWriter &operator=(const CodeWriter &) = delete;
    CodeWriter &operator=(CodeWriter &&) = default;

public:
    bool writeCode(const Module *module, const QStringList &lines);

private:
    WriteDestMap m_dests;
};
