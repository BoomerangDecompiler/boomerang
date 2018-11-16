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


#include "boomerang/ifc/IDecoder.h"
#include "boomerang/ssl/RTLInstDict.h"


namespace cs
{
#include <capstone/capstone.h>
}


/**
 * Base class for instruction decoders using Capstone for disassembling instructions.
 */
class CapstoneDecoder : public IDecoder
{
public:
    /**
     * \param prog the program being decompiled.
     * \param arch Capstone architecture. Usually this is set in the constructor
     *             of the derived class.
     * \param mode Capstone dsassembly mode. Usually this is set in the constructor
     *             of the derived class.
     * \param sslFileName Path to the ssl file holding the instruction semantics,
     *                    relative to the data directory.
     *                    If settings.sslFile is not empty, \p sslFileName is ignored.
     */
    CapstoneDecoder(Prog *prog, cs::cs_arch arch, cs::cs_mode mode, const QString &sslFileName);
    virtual ~CapstoneDecoder();

protected:
    bool isInstructionInGroup(const cs::cs_insn *instruction, uint8_t group);

protected:
    cs::csh m_handle;
    Prog *m_prog;
    RTLInstDict m_dict;
    bool m_debugMode;
};
