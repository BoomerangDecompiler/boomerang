#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CapstoneX86Decoder.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/plugin/Plugin.h"
#include "boomerang/db/Prog.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/util/log/Log.h"


#define X86_MAX_INSTRUCTION_LENGTH (15)


// only map those registers that are mapped to a number
// different from -1 in the SSL file.
// not all registers supported by capstone
static std::map<cs::x86_reg, RegNum> oldRegMap = {
    { cs::X86_REG_INVALID, RegNumSpecial },
    { cs::X86_REG_EAX, REG_X86_EAX },
    { cs::X86_REG_ECX, REG_X86_ECX },
    { cs::X86_REG_EDX, REG_X86_EDX },
    { cs::X86_REG_EBX, REG_X86_EBX },
    { cs::X86_REG_ESP, REG_X86_ESP },
    { cs::X86_REG_EBP, REG_X86_EBP },
    { cs::X86_REG_ESI, REG_X86_ESI },
    { cs::X86_REG_EDI, REG_X86_EDI },
    { cs::X86_REG_AX, REG_X86_AX },
    { cs::X86_REG_CX, REG_X86_CX },
    { cs::X86_REG_DX, REG_X86_DX },
    { cs::X86_REG_BX, REG_X86_BX },
    { cs::X86_REG_SP, REG_X86_SP },
    { cs::X86_REG_BP, REG_X86_BP },
    { cs::X86_REG_SI, REG_X86_SI },
    { cs::X86_REG_DI, REG_X86_DI },
    { cs::X86_REG_AL, REG_X86_AL },
    { cs::X86_REG_CL, REG_X86_CL },
    { cs::X86_REG_DL, REG_X86_DL },
    { cs::X86_REG_BL, REG_X86_BL },
    { cs::X86_REG_AH, REG_X86_AH },
    { cs::X86_REG_CH, REG_X86_CH },
    { cs::X86_REG_DH, REG_X86_DH },
    { cs::X86_REG_BH, REG_X86_BH },
    { cs::X86_REG_CS, REG_X86_CS },
    { cs::X86_REG_DS, REG_X86_DS },
    { cs::X86_REG_ES, REG_X86_ES },
    { cs::X86_REG_FS, REG_X86_FS },
    { cs::X86_REG_GS, REG_X86_GS },
    { cs::X86_REG_SS, REG_X86_SS },
    { cs::X86_REG_IP, RegNumSpecial },
    { cs::X86_REG_ST0, REG_X86_ST0 },
    { cs::X86_REG_ST1, REG_X86_ST1 },
    { cs::X86_REG_ST2, REG_X86_ST2 },
    { cs::X86_REG_ST3, REG_X86_ST3 },
    { cs::X86_REG_ST4, REG_X86_ST4 },
    { cs::X86_REG_ST5, REG_X86_ST5 },
    { cs::X86_REG_ST6, REG_X86_ST6 },
    { cs::X86_REG_ST7, REG_X86_ST7 }
    // fsw / fstp / fcw ?
};


/**
 * Translates Capstone register IDs to Boomerang internal register IDs.
 * \returns RegNumSpecial if register not found.
 */
RegNum fixRegNum(int csRegID)
{
    auto it = oldRegMap.find((cs::x86_reg)csRegID);
    return (it != oldRegMap.end()) ? it->second : RegNumSpecial;
}


/**
 * Converts an operand of an instruction (like %eax + 4*%esi + 0x10)
 * into a SSL expression that can be recognized by the decompiler.
 */
SharedExp operandToExp(const cs::cs_x86_op &operand)
{
    switch (operand.type) {
    case cs::X86_OP_MEM: {
        SharedExp base = (operand.mem.base != cs::X86_REG_INVALID)
                             ? Location::regOf(fixRegNum(operand.mem.base))
                             : Const::get(0);
        SharedExp index = (operand.mem.index != cs::X86_REG_INVALID)
                              ? Location::regOf(fixRegNum(operand.mem.index))
                              : Const::get(0);
        SharedExp scale = Const::get(operand.mem.scale);

        // addrExp = base + index * scale
        SharedExp addrExp = Binary::get(opPlus, base, Binary::get(opMult, scale, index));

        // for some reason the address expression is not simplified
        // if disp == 0
        if (operand.mem.disp != 0) {
            const int64_t disp = operand.mem.disp;

            SharedExp dispExp = Const::get(Address(disp));
            addrExp           = Binary::get(opPlus, addrExp, dispExp);
        }

        return Location::memOf(addrExp)->simplify();
    }
    case cs::X86_OP_REG: {
        return Location::regOf(fixRegNum(operand.reg));
    }
    case cs::X86_OP_IMM: {
        return Const::get(Address(operand.imm));
    }
    default: LOG_ERROR("Unknown x86 instruction operand type %1", (int)operand.type); break;
    }
    return nullptr;
}


CapstoneX86Decoder::CapstoneX86Decoder(Project *project)
    : CapstoneDecoder(project, cs::CS_ARCH_X86, cs::CS_MODE_32, "ssl/x86.ssl")
{
    m_insn = cs::cs_malloc(m_handle);
}


CapstoneX86Decoder::~CapstoneX86Decoder()
{
    cs::cs_free(m_insn, 1);
}


bool CapstoneX86Decoder::initialize(Project *project)
{
    if (!CapstoneDecoder::initialize(project)) {
        return false;
    }

    const int bitness = project->getLoadedBinaryFile()->getBitness();
    switch (bitness) {
    case 16: cs::cs_option(m_handle, cs::CS_OPT_MODE, cs::CS_MODE_16); break;
    case 32: cs::cs_option(m_handle, cs::CS_OPT_MODE, cs::CS_MODE_32); break;
    case 64: cs::cs_option(m_handle, cs::CS_OPT_MODE, cs::CS_MODE_64); break;
    default: break;
    }

    return true;
}


bool CapstoneX86Decoder::decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result)
{
    const Byte *instructionData = reinterpret_cast<const Byte *>((HostAddress(delta) + pc).value());
    size_t size                 = X86_MAX_INSTRUCTION_LENGTH;
    uint64 addr                 = pc.value();

    result.valid = cs_disasm_iter(m_handle, &instructionData, &size, &addr, m_insn);

    if (!result.valid) {
        return false;
    }
    else if (m_insn->id == cs::X86_INS_BSF || m_insn->id == cs::X86_INS_BSR) {
        // special hack to give BSF/BSR the correct semantics since SSL does not support loops yet
        const bool ok = genBSFR(pc, m_insn, result);
        return ok;
    }

    result.iclass   = IClass::NOP; //< ICLASS is irrelevant for x86
    result.numBytes = m_insn->size;
    result.reDecode = false;
    result.rtl      = createRTLForInstruction(pc, m_insn);
    result.valid    = (result.rtl != nullptr);
    return true;
}


QString CapstoneX86Decoder::getRegNameByNum(RegNum regNum) const
{
    return m_dict.getRegDB()->getRegNameByNum(regNum);
}


int CapstoneX86Decoder::getRegSizeByNum(RegNum regNum) const
{
    return m_dict.getRegDB()->getRegSizeByNum(regNum);
}


static const QString operandNames[] = {
    "",    // X86_OP_INVALID
    "reg", // X86_OP_REG
    "imm", // X86_OP_IMM
    "rm",  // X86_OP_MEM
};

std::unique_ptr<RTL> CapstoneX86Decoder::createRTLForInstruction(Address pc,
                                                                 const cs::cs_insn *instruction)
{
    const int numOperands         = instruction->detail->x86.op_count;
    const cs::cs_x86_op *operands = instruction->detail->x86.operands;

    const QString insnID     = getInstructionID(instruction);
    std::unique_ptr<RTL> rtl = instantiateRTL(pc, qPrintable(insnID), numOperands, operands);
    if (!rtl) {
        LOG_ERROR("Cannot find semantics for instruction '%1' at address %2, "
                  "treating instruction as NOP",
                  insnID, pc);
        return instantiateRTL(pc, "NOP", 0, nullptr);
    }

    if (isInstructionInGroup(instruction, cs::CS_GRP_CALL)) {
        auto it = std::find_if(rtl->rbegin(), rtl->rend(),
                               [](const SharedConstStmt &stmt) { return stmt->isCall(); });

        if (it != rtl->rend()) {
            std::shared_ptr<CallStatement> call = (*it)->as<CallStatement>();

            if (!call->isComputed()) {
                const SharedConstExp &callDest = call->getDest();
                const Address destAddr         = callDest->access<Const>()->getAddr();

                if (destAddr == pc + 5) {
                    // call to next instruction (just pushes instruction pointer to stack)
                    // delete the call statement
                    rtl->erase(std::next(it).base());
                }
                else {
                    Function *destProc = m_prog->getOrCreateFunction(destAddr);

                    if (destProc == reinterpret_cast<Function *>(-1)) {
                        destProc = nullptr;
                    }

                    call->setDestProc(destProc);
                }
            }
        }
    }
    else if (isInstructionInGroup(instruction, cs::X86_GRP_JUMP)) {
        if (rtl->back()->isBranch()) {
            std::shared_ptr<BranchStatement> branch = rtl->back()->as<BranchStatement>();
            const bool isComputedJump               = !branch->getDest()->isIntConst();

            BranchType bt = BranchType::INVALID;
            switch (instruction->id) {
            case cs::X86_INS_JE: bt = BranchType::JE; break;
            case cs::X86_INS_JNE: bt = BranchType::JNE; break;
            case cs::X86_INS_JL: bt = BranchType::JSL; break; // signed less
            case cs::X86_INS_JLE: bt = BranchType::JSLE; break;
            case cs::X86_INS_JGE: bt = BranchType::JSGE; break;
            case cs::X86_INS_JG: bt = BranchType::JSG; break;
            case cs::X86_INS_JB: bt = BranchType::JUL; break; // unsigned less
            case cs::X86_INS_JBE: bt = BranchType::JULE; break;
            case cs::X86_INS_JAE: bt = BranchType::JUGE; break;
            case cs::X86_INS_JA: bt = BranchType::JUG; break;
            case cs::X86_INS_JS: bt = BranchType::JMI; break;
            case cs::X86_INS_JNS: bt = BranchType::JPOS; break;
            case cs::X86_INS_JO: bt = BranchType::JOF; break;
            case cs::X86_INS_JNO: bt = BranchType::JNOF; break;
            case cs::X86_INS_JP: bt = BranchType::JPAR; break;
            case cs::X86_INS_JNP: bt = BranchType::JNPAR; break;
            case cs::X86_INS_JCXZ: bt = BranchType::JE; break;
            case cs::X86_INS_JECXZ: bt = BranchType::JE; break; // because jmp condition is %ecx = 0
            case cs::X86_INS_LOOP: bt = BranchType::JNE; break;
            case cs::X86_INS_LOOPE: bt = BranchType::JE; break;
            case cs::X86_INS_LOOPNE: bt = BranchType::JNE; break;
            default: assert(false); break;
            }

            branch->setCondType(bt, false);
            branch->setIsComputed(isComputedJump);

            // Need to fix up the conditional expression here...
            // setCondType() assigns the wrong expression for jumps that do not depend on flags
            switch (instruction->id) {
            case cs::X86_INS_JCXZ: {
                branch->setCondExpr(
                    Binary::get(opEquals, Location::regOf(REG_X86_CX), Const::get(0)));
                break;
            }
            case cs::X86_INS_JECXZ: {
                branch->setCondExpr(
                    Binary::get(opEquals, Location::regOf(REG_X86_ECX), Const::get(0)));
                break;
            }
            case cs::X86_INS_LOOP: {
                // FIXME wrong for 16 bit programs
                branch->setCondExpr(
                    Binary::get(opNotEqual, Location::regOf(REG_X86_ECX), Const::get(0)));
                break;
            }
            case cs::X86_INS_LOOPE: {
                // FIXME wrong for 16 bit programs
                // clang-format off
                branch->setCondExpr(Binary::get(opAnd,
                                                Binary::get(opNotEqual,
                                                            Location::regOf(REG_X86_ECX),
                                                            Const::get(0)),
                                                Binary::get(opEquals,
                                                            Terminal::get(opZF),
                                                            Const::get(1))));
                // clang-format on
                break;
            }
            case cs::X86_INS_LOOPNE: {
                // FIXME wrong for 16 bit programs
                // clang-format off
                branch->setCondExpr(Binary::get(opAnd,
                                                Binary::get(opNotEqual,
                                                            Location::regOf(REG_X86_ECX),
                                                            Const::get(0)),
                                                Binary::get(opEquals,
                                                            Terminal::get(opZF),
                                                            Const::get(0))));
                // clang-format on
                break;
            }
            default: break;
            }
        }
    }
    else if (insnID.startsWith("SET")) {
        std::shared_ptr<BoolAssign> bas(new BoolAssign(8));
        bas->setCondExpr(rtl->front()->as<Assign>()->getRight()->clone());
        bas->setLeft(rtl->front()->as<Assign>()->getLeft()->clone());

        BranchType bt = BranchType::INVALID;
        switch (instruction->id) {
        case cs::X86_INS_SETE: bt = BranchType::JE; break;
        case cs::X86_INS_SETNE: bt = BranchType::JNE; break;
        case cs::X86_INS_SETL: bt = BranchType::JSL; break; // signed less
        case cs::X86_INS_SETLE: bt = BranchType::JSLE; break;
        case cs::X86_INS_SETGE: bt = BranchType::JSGE; break;
        case cs::X86_INS_SETG: bt = BranchType::JSG; break;
        case cs::X86_INS_SETB: bt = BranchType::JUL; break; // unsigned less
        case cs::X86_INS_SETBE: bt = BranchType::JULE; break;
        case cs::X86_INS_SETAE: bt = BranchType::JUGE; break;
        case cs::X86_INS_SETA: bt = BranchType::JUG; break;
        case cs::X86_INS_SETS: bt = BranchType::JMI; break;
        case cs::X86_INS_SETNS: bt = BranchType::JPOS; break;
        case cs::X86_INS_SETO: bt = BranchType::JOF; break;
        case cs::X86_INS_SETNO: bt = BranchType::JNOF; break;
        case cs::X86_INS_SETP: bt = BranchType::JPAR; break;
        case cs::X86_INS_SETNP: bt = BranchType::JNPAR; break;
        default: assert(false); break;
        }

        bas->setCondType(bt);
        if (rtl->size() > 1) {
            LOG_WARN(
                "%1 additional statements in RTL for instruction '%2'; results may be inaccurate",
                rtl->size() - 1, insnID);
        }

        rtl->clear();
        rtl->append(bas);
    }

    return rtl;
}


std::unique_ptr<RTL> CapstoneX86Decoder::instantiateRTL(Address pc, const char *instructionID,
                                                        int numOperands,
                                                        const cs::cs_x86_op *operands)
{
    // Take the argument, convert it to upper case and remove any .'s
    const QString sanitizedName = QString(instructionID).remove(".").toUpper();

    std::vector<SharedExp> args(numOperands);
    for (int i = 0; i < numOperands; i++) {
        args[i] = operandToExp(operands[i]);
    }

    if (m_debugMode) {
        QString argNames;
        for (int i = 0; i < numOperands; i++) {
            if (i != 0) {
                argNames += " ";
            }
            argNames += args[i]->toString();
        }

        LOG_MSG("Instantiating RTL at %1: %2 %3", pc, instructionID, argNames);
    }

    return m_dict.instantiateRTL(sanitizedName, pc, args);
}


bool CapstoneX86Decoder::genBSFR(Address pc, const cs::cs_insn *instruction, DecodeResult &result)
{
    // Note the horrible hack needed here. We need initialisation code, and an extra branch, so the
    // %SKIP/%RPT won't work. We need to emit 6 statements, but these need to be in 3 RTLs, since
    // the destination of a branch has to be to the start of an RTL.  So we use a state machine, and
    // set numBytes to 0 for the first two times. That way, this instruction ends up emitting three
    // RTLs, each with the semantics we need. Note: we don't use x86.ssl for these.
    //
    // BSFR1:
    //    pc+0:    *1* zf := 1
    //    pc+0:    goto exit if src = 0
    // BSFR2:
    //    pc+1:    *1* zf := 0
    //    pc+1:    dest := 0 if BSF else dest := opsize - 1
    // BSFR3:
    //    pc+2:    dest := dest op 1
    //    pc+2:    goto pc+2 if src@[dest:dest]=0
    // exit:
    //

    std::shared_ptr<BranchStatement> b = nullptr;
    result.rtl                         = std::unique_ptr<RTL>(new RTL(pc + m_bsfrState));

    const cs::cs_x86_op &dstOp = instruction->detail->x86.operands[0];
    const cs::cs_x86_op &srcOp = instruction->detail->x86.operands[1];

    assert(dstOp.size == srcOp.size);
    const SharedExp dest = operandToExp(dstOp);
    const SharedExp src  = operandToExp(srcOp);
    const int size       = dstOp.size * 8;
    const int init       = instruction->id == cs::X86_INS_BSF ? 0 : size - 1;
    const OPER incdec    = instruction->id == cs::X86_INS_BSF ? opPlus : opMinus;

    switch (m_bsfrState) {
    case 0:
        result.rtl->append(
            std::make_shared<Assign>(IntegerType::get(1), Terminal::get(opZF), Const::get(1)));
        b.reset(new BranchStatement);
        b->setDest(pc + instruction->size);
        b->setCondType(BranchType::JE);
        b->setCondExpr(Binary::get(opEquals, src->clone(), Const::get(0)));
        result.rtl->append(b);
        break;

    case 1:
        result.rtl->append(
            std::make_shared<Assign>(IntegerType::get(1), Terminal::get(opZF), Const::get(0)));
        result.rtl->append(
            std::make_shared<Assign>(IntegerType::get(size), dest->clone(), Const::get(init)));
        break;

    case 2:
        result.rtl->append(
            std::make_shared<Assign>(IntegerType::get(size), dest->clone(),
                                     Binary::get(incdec, dest->clone(), Const::get(1))));
        b.reset(new BranchStatement);
        b->setDest(pc + 2);
        b->setCondType(BranchType::JE);
        b->setCondExpr(Binary::get(opEquals,
                                   Ternary::get(opAt, src->clone(), dest->clone(), dest->clone()),
                                   Const::get(0)));
        result.rtl->append(b);
        break;

    default:
        // Should never happen
        LOG_FATAL("Unknown BSFR state %1", m_bsfrState);
    }

    // Keep numBytes == 0 until the last state, so we re-decode this instruction 3 times
    if (m_bsfrState != 3 - 1) {
        // Let the number of bytes be 1. This is important at least for setting the fallthrough
        // address for the branch (in the first RTL), which should point to the next RTL
        result.numBytes = 1;
        result.reDecode = true; // Decode this instuction again
    }
    else {
        result.numBytes = instruction->size;
        result.reDecode = false;
    }

    if (m_debugMode) {
        LOG_MSG("%1: BS%2%3%4", pc + m_bsfrState, (init == -1 ? "F" : "R"),
                (size == 32 ? ".od" : ".ow"), m_bsfrState + 1);
    }

    if (++m_bsfrState == 3) {
        m_bsfrState = 0; // Ready for next time
    }

    return true;
}


QString CapstoneX86Decoder::getInstructionID(const cs::cs_insn *instruction) const
{
    const int numOperands         = instruction->detail->x86.op_count;
    const cs::cs_x86_op *operands = instruction->detail->x86.operands;

    // clang-format off
    if (instruction->id == cs::X86_INS_AND &&
        operands[0].type == cs::X86_OP_REG && operands[0].reg == cs::X86_REG_ESP &&
        operands[1].type == cs::X86_OP_IMM && operands[1].imm == 0xFFFFFFF0) {
        // special hack to ignore 'and esp, 0xfffffff0' in startup code
        return "NOP";
    }
    // clang-format on

    QString insnID = cs::cs_insn_name(m_handle, instruction->id);

    switch (instruction->detail->x86.prefix[0]) {
    case cs::X86_PREFIX_REP: insnID = "REP" + insnID; break;
    case cs::X86_PREFIX_REPNE: insnID = "REPNE" + insnID; break;
    }

    insnID = insnID.toUpper();

    for (int i = 0; i < numOperands; i++) {
        // example: ".imm8"
        QString operandName = "." + operandNames[operands[i].type] +
                              QString::number(operands[i].size * 8);

        insnID += operandName;
    }

    return insnID;
}


BOOMERANG_DEFINE_PLUGIN(PluginType::Decoder, CapstoneX86Decoder, "Capstone x86 decoder plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
