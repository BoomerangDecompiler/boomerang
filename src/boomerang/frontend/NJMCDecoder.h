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


#include "boomerang/core/Settings.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/RTLInstDict.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/BoolAssign.h"

#include "boomerang/util/Types.h"
#include "boomerang/ifc/IDecoder.h"


class BinaryImage;


/**
 * The NJMCDecoder class is a class that contains NJMC generated decoding methods.
 */
class NJMCDecoder : public IDecoder
{
public:
    /// \param       prog Pointer to the Prog object
    NJMCDecoder(Prog *prog, const QString& sslFilePath);
    NJMCDecoder(const NJMCDecoder& other) = delete;
    NJMCDecoder(NJMCDecoder&& other) = default;

    /// \copydoc IDecoder::~IDecoder
    virtual ~NJMCDecoder() override = default;

    NJMCDecoder& operator=(const NJMCDecoder& other) = delete;
    NJMCDecoder& operator=(NJMCDecoder&& other) = default;

public:
    RTLInstDict& getRTLDict() { return m_rtlDict; }

    /**
     * Process an indirect jump instruction.
     * \param   name name of instruction (for debugging)
     * \param   size size of instruction in bytes
     * \param   dest destination Exp*
     * \param   pc native pc
     * \param   stmts list of statements (?)
     * \param   result ref to decoder result object
     */
    void processComputedJump(const char *name, int size, SharedExp dest, Address pc, DecodeResult& result);

    /**
     * Process an indirect call instruction.
     * \param   name name of instruction (for debugging)
     * \param   size size of instruction in bytes
     * \param   dest destination Exp*
     * \param   pc native pc
     * \param   result ref to decoder result object
     */
    void processComputedCall(const char *name, int size, SharedExp dest, Address pc, DecodeResult& result);

    /// \copydoc IInstructionTranslator::getRegName
    QString getRegName(int idx) const override;

    /// \copydoc IInstructionTranslator::getRegSize
    int getRegSize(int idx) const override;

    /// \copydoc IInstructionTranslator::getRegIdx
    int getRegIdx(const QString& name) const override;

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
    std::unique_ptr<RTL> instantiate(Address pc, const char *name, const std::initializer_list<SharedExp>& args = {});

    /**
     * Similarly to \ref NJMCDecoder::instantiate, given a parameter name
     * and a list of Exp*'s representing sub-parameters, return
     * a fully substituted Exp for the whole expression
     *
     * \param   name  parameter name
     *          ...   Exp* representing actual operands
     * \returns an instantiated list of Exps
     */
    SharedExp instantiateNamedParam(char *name, const std::initializer_list<SharedExp>& args);

    /**
     * In the event that it's necessary to synthesize the call of a
     * named parameter generated with \ref instantiateNamedParam, this
     * \ref substituteCallArgs will substitute the arguments that follow into
     * the expression.
     *
     * \note    Should only be used after instantiateNamedParam(name, ..);
     * \note    exp (the pointer) could be changed
     *
     * \param   name  parameter name
     * \param   exp   expression to instantiate into
     * \param   args Exp* representing actual operands
     */
    void substituteCallArgs(char *name, SharedExp *exp, const std::initializer_list<SharedExp>& args);

    /**
     * Process an unconditional jump instruction
     * Also check if the destination is a label (MVE: is this done?)
     *
     * \param   name name of instruction (for debugging)
     * \param   size size of instruction in bytes
     * \param   pc native pc
     * \param   result ref to decoder result object
     */
    void processUnconditionalJump(const char *name, int size, HostAddress relocd, ptrdiff_t delta, Address pc,
                                  DecodeResult& result);


    /**
     * Converts a numbered register to a suitable expression.
     * \param   regNum - the register number, e.g. 0 for eax
     * \returns the Exp* for the register NUMBER (e.g. "int 36" for %f4)
     */
    SharedExp dis_Reg(int regNum);

    /**
     * Converts a number to a Exp* expression.
     * \param        num - a number
     * \returns      the Exp* representation of the given number
     */
    SharedExp dis_Num(unsigned num);

protected:
    // Dictionary of instruction patterns, and other information summarised from the SSL file
    // (e.g. source machine's endianness)
    RTLInstDict m_rtlDict;
    Prog *m_prog = nullptr;
    BinaryImage *m_image = nullptr;
};


/**
 * These are the macros that each of the .m files depend upon.
 */

#define SHOW_ASM(output)               \
    if (m_prog->getProject()->getSettings()->debugDecoder) {               \
        QString asmStr;                \
        QTextStream ost(&asmStr);      \
        ost << output;                 \
        LOG_MSG("%1: %2", pc, asmStr); \
    }

/*
 * addresstoPC returns the raw number as the address.  PC could be an
 * abstract type, in our case, PC is the raw address.
 */
#define addressToPC(pc)    pc

// Macros for branches. Note: don't put inside a "match" statement, since
// the ordering is changed and multiple copies may be made

#define COND_JUMP(name, size, relocd, cond)                                    \
    BranchStatement *jump = new BranchStatement;                               \
    result.rtl->append(jump);                                                  \
    result.numBytes = size;                                                    \
    jump->setDest(Address(relocd.value() - Util::signExtend<int64_t>(delta))); \
    jump->setCondType(cond);                                                   \
    SHOW_ASM(name << " " << relocd)

// This one is X86 specific
#define SETS(name, dest, cond)                          \
    BoolAssign * bs = new BoolAssign(8);                \
    bs->setLeftFromList(result.rtl->getStatements());   \
    result.rtl->clear();                                \
    result.rtl->append(bs);                             \
    bs->setCondType(cond);                              \
    result.numBytes = 3;                                \
    SHOW_ASM(name << " " << dest)
