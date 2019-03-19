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
#include "boomerang/ssl/exp/ExpHelp.h"
#include "boomerang/util/Util.h"


class BinaryImage;


/**
 * The NJMCDecoder class is a class that contains NJMC generated decoding methods.
 */
class BOOMERANG_API NJMCDecoder : public IDecoder
{
public:
    /**
     * \param sslFilePath Path to the ssl file, relative to the data directory.
     *                    If settings.sslFile is not empty, \p sslFilePath is ignored.
     */
    NJMCDecoder(Project *project, const QString &sslFilePath);
    NJMCDecoder(const NJMCDecoder &other) = delete;
    NJMCDecoder(NJMCDecoder &&other)      = default;

    /// \copydoc IDecoder::~IDecoder
    virtual ~NJMCDecoder() override = default;

    NJMCDecoder &operator=(const NJMCDecoder &other) = delete;
    NJMCDecoder &operator=(NJMCDecoder &&other) = default;

public:
    bool initialize(Project *project) override;

    const RTLInstDict *getDict() const override { return &m_rtlDict; }

    /// \copydoc IDecoder::getRegName
    QString getRegNameByNum(RegNum num) const override;

    /// \copydoc IDecoder::getRegSize
    int getRegSizeByNum(RegNum num) const override;

    /// \copydoc IDecoder::getRegNumByName
    RegNum getRegNumByName(const QString &name) const override;

    // only implemented for SPARC
    virtual bool isRestore(HostAddress hostPC);

protected:
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

protected:
    // Dictionary of instruction patterns, and other information summarised from the SSL file
    // (e.g. source machine's endianness)
    RTLInstDict m_rtlDict;
    Prog *m_prog         = nullptr;
    BinaryImage *m_image = nullptr;
};
