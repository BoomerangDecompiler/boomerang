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

#include <cstring>


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
    // check that all required registers are present
    if (m_dict.getRegDB()->getRegNameByNum(REG_X86_ESP).isEmpty()) {
        throw std::runtime_error("Required register #28 (%esp) not present");
    }

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


bool CapstoneX86Decoder::disassembleInstruction(Address pc, ptrdiff_t delta,
                                                MachineInstruction &result)
{
    const Byte *instructionData = reinterpret_cast<const Byte *>((HostAddress(delta) + pc).value());
    size_t size                 = X86_MAX_INSTRUCTION_LENGTH;
    uint64 addr                 = pc.value();

    const bool valid = cs_disasm_iter(m_handle, &instructionData, &size, &addr, m_insn);

    if (!valid) {
        return false;
    }

    result.m_addr = Address(m_insn->address);
    result.m_id   = m_insn->id;
    result.m_size = m_insn->size;

    std::strncpy(result.m_mnem.data(), m_insn->mnemonic, MNEM_SIZE);
    std::strncpy(result.m_opstr.data(), m_insn->op_str, OPSTR_SIZE);
    result.m_mnem[MNEM_SIZE - 1]   = '\0';
    result.m_opstr[OPSTR_SIZE - 1] = '\0';

    const std::size_t numOperands = m_insn->detail->x86.op_count;
    result.m_operands.resize(numOperands);

    for (std::size_t i = 0; i < numOperands; ++i) {
        result.m_operands[i] = operandToExp(m_insn->detail->x86.operands[i]);
    }

    result.m_templateName = getTemplateName(m_insn);

    result.setGroup(MIGroup::Jump, isInstructionInGroup(m_insn, cs::CS_GRP_JUMP));
    result.setGroup(MIGroup::Call, isInstructionInGroup(m_insn, cs::CS_GRP_CALL));
    result.setGroup(MIGroup::BoolAsgn, result.m_templateName.startsWith("SET"));
    result.setGroup(MIGroup::Ret, isInstructionInGroup(m_insn, cs::CS_GRP_RET) ||
                                      isInstructionInGroup(m_insn, cs::CS_GRP_IRET));

    if (result.isInGroup(MIGroup::Jump) || result.isInGroup(MIGroup::Call)) {
        assert(result.getNumOperands() > 0);
        result.setGroup(MIGroup::Computed, !result.m_operands[0]->isConst());
    }

    return true;
}


bool CapstoneX86Decoder::liftInstruction(const MachineInstruction &insn, LiftedInstruction &lifted)
{
    if (insn.m_id == cs::X86_INS_BSF || insn.m_id == cs::X86_INS_BSR) {
        // special hack to give BSF/BSR the correct semantics since SSL does not support loops yet
        const bool ok = genBSFR(insn, lifted);
        return ok;
    }

    // clang-format off
    if (insn.m_id == cs::X86_INS_AND &&
        *insn.m_operands[0] == *Location::regOf(REG_X86_ESP) &&
        *insn.m_operands[1] == *Const::get(Address(0xFFFFFFF0U))) {

        // special hack to ignore 'and esp, 0xfffffff0' in startup code
        lifted.addPart(std::make_unique<RTL>(insn.m_addr));
    }
    // clang-format on
    else {
        lifted.addPart(createRTLForInstruction(insn));
    }

    return lifted.getFirstRTL() != nullptr;
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

std::unique_ptr<RTL> CapstoneX86Decoder::createRTLForInstruction(const MachineInstruction &insn)
{
    const QString insnID     = insn.m_templateName;
    std::unique_ptr<RTL> rtl = instantiateRTL(insn);

    if (!rtl) {
        return nullptr;
    }

    if (insn.isInGroup(MIGroup::Call)) {
        auto it = std::find_if(rtl->rbegin(), rtl->rend(),
                               [](const SharedConstStmt &stmt) { return stmt->isCall(); });

        if (it != rtl->rend()) {
            std::shared_ptr<CallStatement> call = (*it)->as<CallStatement>();

            if (!call->isComputed()) {
                const SharedConstExp &callDest = call->getDest();
                const Address destAddr         = callDest->access<Const>()->getAddr();

                if (destAddr == insn.m_addr + 5) {
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
    else if (insn.isInGroup(MIGroup::Jump)) {
        if (rtl->back()->isBranch()) {
            std::shared_ptr<BranchStatement> branch = rtl->back()->as<BranchStatement>();
            const bool isComputedJump               = !branch->getDest()->isIntConst();

            BranchType bt = BranchType::INVALID;
            switch (insn.m_id) {
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
            switch (insn.m_id) {
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
    else if (insn.isInGroup(MIGroup::BoolAsgn)) {
        std::shared_ptr<BoolAssign> bas(new BoolAssign(8));
        bas->setCondExpr(rtl->front()->as<Assign>()->getRight()->clone());
        bas->setLeft(rtl->front()->as<Assign>()->getLeft()->clone());

        BranchType bt = BranchType::INVALID;
        switch (insn.m_id) {
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


std::unique_ptr<RTL> CapstoneX86Decoder::instantiateRTL(const MachineInstruction &insn)
{
    // Take the argument, convert it to upper case and remove any .'s
    const QString sanitizedName   = QString(insn.m_templateName).remove(".").toUpper();
    const std::size_t numOperands = insn.getNumOperands();

    if (m_debugMode) {
        QString argNames;
        for (std::size_t i = 0; i < numOperands; i++) {
            if (i != 0) {
                argNames += " ";
            }
            argNames += insn.m_operands[i]->toString();
        }

        LOG_MSG("Instantiating RTL at %1: %2 %3", insn.m_addr, insn.m_templateName, argNames);
    }

    return m_dict.instantiateRTL(sanitizedName, insn.m_addr, insn.m_operands);
}


bool CapstoneX86Decoder::genBSFR(const MachineInstruction &insn, LiftedInstruction &result)
{
    // Note the horrible hack needed here. We need initialisation code, and an extra branch, so the
    // %SKIP/%RPT won't work. We need to emit 6 statements, but these need to be in 3 RTLs, since
    // the destination of a branch has to be to the start of an RTL.
    // Note: we don't use x86.ssl for these.
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

    if (m_debugMode) {
        const std::size_t numOperands = insn.getNumOperands();
        QString argNames;
        for (std::size_t i = 0; i < numOperands; i++) {
            if (i != 0) {
                argNames += " ";
            }
            argNames += insn.m_operands[i]->toString();
        }

        LOG_MSG("Instantiating RTL at %1: %2 %3", insn.m_addr, insn.m_templateName, argNames);
    }

    const SharedExp dest   = insn.m_operands[0];
    const SharedExp src    = insn.m_operands[1];
    const std::size_t size = dest->isRegOfConst()
                                 ? getRegSizeByNum(dest->access<Const, 1>()->getInt())
                                 : getRegSizeByNum(src->access<Const, 1>()->getInt());

    assert(size > 0);

    const int init    = insn.m_id == cs::X86_INS_BSF ? 0 : size - 1;
    const OPER incdec = insn.m_id == cs::X86_INS_BSF ? opPlus : opMinus;

    LiftedInstructionPart *rtls[4];

    // first RTL
    {
        std::unique_ptr<RTL> rtl(new RTL(insn.m_addr + 0));

        rtl->append(
            std::make_shared<Assign>(IntegerType::get(1), Terminal::get(opZF), Const::get(1)));
        std::shared_ptr<BranchStatement> b(new BranchStatement);
        b->setDest(insn.m_addr + insn.m_size);
        b->setCondType(BranchType::JE);
        b->setCondExpr(Binary::get(opEquals, src->clone(), Const::get(0)));
        rtl->append(b);

        rtls[0] = result.addPart(std::move(rtl));
    }

    // second RTL
    {
        std::unique_ptr<RTL> rtl(new RTL(insn.m_addr + 1));

        rtl->append(
            std::make_shared<Assign>(IntegerType::get(1), Terminal::get(opZF), Const::get(0)));
        rtl->append(
            std::make_shared<Assign>(IntegerType::get(size), dest->clone(), Const::get(init)));

        rtls[1] = result.addPart(std::move(rtl));
    }

    // third RTL
    {
        std::unique_ptr<RTL> rtl(new RTL(insn.m_addr + 2));

        rtl->append(std::make_shared<Assign>(IntegerType::get(size), dest->clone(),
                                             Binary::get(incdec, dest->clone(), Const::get(1))));
        std::shared_ptr<BranchStatement> b(new BranchStatement);
        b->setDest(insn.m_addr + 2);
        b->setCondType(BranchType::JE);
        b->setCondExpr(Binary::get(opEquals,
                                   Ternary::get(opAt, src->clone(), dest->clone(), dest->clone()),
                                   Const::get(0)));
        rtl->append(b);

        rtls[2] = result.addPart(std::move(rtl));
    }

    // fourth (empty) RTL
    {
        std::unique_ptr<RTL> rtl(new RTL(insn.m_addr + 3));
        rtls[3] = result.addPart(std::move(rtl));
    }

    result.addEdge(rtls[0], rtls[3]);
    result.addEdge(rtls[0], rtls[1]);

    result.addEdge(rtls[1], rtls[2]);

    result.addEdge(rtls[2], rtls[2]);
    result.addEdge(rtls[2], rtls[3]);

    return true;
}


QString CapstoneX86Decoder::getTemplateName(const cs::cs_insn *instruction) const
{
    const int numOperands         = instruction->detail->x86.op_count;
    const cs::cs_x86_op *operands = instruction->detail->x86.operands;

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
