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
#include "boomerang/ifc/IDecoder.h"
#include "boomerang/ssl/RTLInstDict.h"
#include "boomerang/ssl/exp/ExpHelp.h"


/**
 * The definition of the instruction decoder for ST20.
 */
class BOOMERANG_PLUGIN_API ST20Decoder : public IDecoder
{
public:
    /// \copydoc IDecoder::IDecoder
    ST20Decoder(Project *project);
    ST20Decoder(const ST20Decoder &other) = delete;
    ST20Decoder(ST20Decoder &&other)      = default;

    virtual ~ST20Decoder() override = default;

    ST20Decoder &operator=(const ST20Decoder &other) = delete;
    ST20Decoder &operator=(ST20Decoder &&other) = default;

public:
    /// \copydoc IDecoder::initialize
    bool initialize(Project *project) override;

    const RTLInstDict *getDict() const override { return &m_rtlDict; }

    /// \copydoc IDecoder::getRegName
    QString getRegNameByNum(RegNum num) const override;

    /// \copydoc IDecoder::getRegSize
    int getRegSizeByNum(RegNum num) const override;

public:
    /// \copydoc IDecoder::decodeInstruction
    virtual bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result) override;

    /// \returns false
    bool isSPARCRestore(Address pc, ptrdiff_t delta) const override;

private:
    /**
     * Given an instruction name and a variable list of expressions
     * representing the actual operands of the instruction,
     * use the RTL template dictionary to return the instantiated RTL
     * representing the semantics of the instruction.
     * This method also displays a disassembly of the instruction if the
     * relevant compilation flag has been set.
     *
     * \param   pc  native PC
     * \param   name - instruction name
     * \param   args Semantic String ptrs representing actual operands
     * \returns an instantiated list of Exps
     */
    std::unique_ptr<RTL> instantiate(Address pc, const char *name,
                                     const std::initializer_list<SharedExp> &args = {});

    /// \param prefixTotal The sum of all prefixes
    /// \returns the name of an instruction determined by its prefixes (e.g. 0x53 -> mul)
    const char *getInstructionName(int prefixTotal) const;

private:
    /// Dictionary of instruction patterns, and other information summarised from the SSL file
    /// (e.g. source machine's endianness)
    RTLInstDict m_rtlDict;
    Prog *m_prog = nullptr;
};
