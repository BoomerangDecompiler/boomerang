#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CapstonePPCDecoder.h"

#include "boomerang/core/plugin/Plugin.h"
#include "boomerang/db/Prog.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/util/log/Log.h"

#include <cstring>


#define PPC_MAX_INSTRUCTION_LENGTH (4)

// only map those registers that are mapped to a number
// different from -1 in the SSL file.
// not all registers supported by capstone
// clang-format off
static std::map<cs::ppc_reg, RegNum> oldRegMap = {
    { cs::PPC_REG_LR, REG_PPC_LR },
    { cs::PPC_REG_CTR, REG_PPC_CTR }
};
// clang-format on


/**
 * Translates Capstone register IDs to Boomerang internal register IDs.
 * \returns RegNumSpecial if register not found.
 */
RegNum fixRegNum(int csRegID)
{
    // GPR
    if (csRegID >= cs::PPC_REG_R0 && csRegID <= cs::PPC_REG_R31) {
        return REG_PPC_G0 + (csRegID - cs::PPC_REG_R0);
    }
    // FPR
    else if (csRegID >= cs::PPC_REG_F0 && csRegID <= cs::PPC_REG_F31) {
        return REG_PPC_F0 + (csRegID - cs::PPC_REG_F0);
    }
    // Vector regs
    else if (csRegID >= cs::PPC_REG_V0 && csRegID <= cs::PPC_REG_V31) {
        return REG_PPC_VR0 + (csRegID - cs::PPC_REG_V0);
    }
    // CR fields
    else if (csRegID >= cs::PPC_REG_CR0 && csRegID <= cs::PPC_REG_CR7) {
        return REG_PPC_CR0 + (csRegID - cs::PPC_REG_CR0);
    }

    auto it = oldRegMap.find((cs::ppc_reg)csRegID);
    return (it != oldRegMap.end()) ? it->second : RegNumSpecial;
}


SharedExp operandToExp(const cs::cs_ppc_op &operand)
{
    switch (operand.type) {
    case cs::PPC_OP_IMM: {
        return Const::get(Address(operand.imm));
    }
    case cs::PPC_OP_REG: {
        return Location::regOf(fixRegNum(operand.reg));
    }
    case cs::PPC_OP_MEM: {
        return Location::memOf(Binary::get(opPlus, Location::regOf(fixRegNum(operand.mem.base)),
                                           Const::get(operand.mem.disp)))
            ->simplifyArith();
    }
    default: LOG_ERROR("Unknown ppc instruction operand type %1", (int)operand.type); break;
    }

    return nullptr;
}


CapstonePPCDecoder::CapstonePPCDecoder(Project *project)
    : CapstoneDecoder(project, cs::CS_ARCH_PPC,
                      (cs::cs_mode)(cs::CS_MODE_32 + cs::CS_MODE_BIG_ENDIAN), "ssl/ppc.ssl")
{
}

bool CapstonePPCDecoder::disassembleInstruction(Address pc, ptrdiff_t delta,
                                                MachineInstruction &result)
{
    const Byte *instructionData = reinterpret_cast<const Byte *>((HostAddress(delta) + pc).value());

    cs::cs_insn *decodedInstruction;
    size_t numInstructions = cs_disasm(m_handle, instructionData, PPC_MAX_INSTRUCTION_LENGTH,
                                       pc.value(), 1, &decodedInstruction);
    const bool valid       = numInstructions > 0;

    if (!valid) {
        return false;
    }

    // Adjust the operands of cr* instructions (e.g. crxor).
    // This is to work around a bug in Capstone: The operands are disassembled as PPC_OP_REG
    // instead of PPC_OP_IMM or PPC_OP_CRX. See https://github.com/aquynh/capstone/issues/971
    // for details.
    if (isCRManip(decodedInstruction)) {
        for (std::size_t i = 0; i < decodedInstruction->detail->ppc.op_count; ++i) {
            cs::cs_ppc_op &operand = decodedInstruction->detail->ppc.operands[i];

            const int bitNum = operand.reg - cs::PPC_REG_R0;
            operand.type     = cs::PPC_OP_IMM;
            operand.imm      = bitNum;
        }
    }

    result.m_addr   = Address(decodedInstruction->address);
    result.m_id     = decodedInstruction->id;
    result.m_size   = decodedInstruction->size;
    result.m_iclass = IClass::NOP; // Irrelevant for PPC

    std::strncpy(result.m_mnem.data(), decodedInstruction->mnemonic, MNEM_SIZE);
    std::strncpy(result.m_opstr.data(), decodedInstruction->op_str, OPSTR_SIZE);
    result.m_mnem[MNEM_SIZE - 1]   = '\0';
    result.m_opstr[OPSTR_SIZE - 1] = '\0';

    const std::size_t numOperands = decodedInstruction->detail->ppc.op_count;
    result.m_operands.resize(numOperands);

    for (std::size_t i = 0; i < numOperands; ++i) {
        result.m_operands[i] = operandToExp(decodedInstruction->detail->ppc.operands[i]);
    }

    result.m_templateName = getTemplateName(decodedInstruction);

    cs_free(decodedInstruction, numInstructions);
    return true;
}


bool CapstonePPCDecoder::liftInstruction(const MachineInstruction &insn, DecodeResult &lifted)
{
    lifted.iclass = IClass::NOP; //< only relevant for architectures with delay slots
    lifted.reLift = false;
    lifted.rtl    = createRTLForInstruction(insn);

    return lifted.rtl != nullptr;
}


QString CapstonePPCDecoder::getRegNameByNum(RegNum regNum) const
{
    return m_dict.getRegDB()->getRegNameByNum(regNum);
}


int CapstonePPCDecoder::getRegSizeByNum(RegNum regNum) const
{
    return m_dict.getRegDB()->getRegSizeByNum(regNum);
}


std::unique_ptr<RTL> CapstonePPCDecoder::createRTLForInstruction(const MachineInstruction &insn)
{
    std::unique_ptr<RTL> rtl = instantiateRTL(insn);

    if (rtl == nullptr) {
        return nullptr;
    }

    const QString insnID          = insn.m_templateName;
    const std::size_t numOperands = insn.getNumOperands();

    if (insnID == "BL" || insnID == "BLA") {
        Address callDest = Address(insn.m_operands[0]->access<Const>()->getLong());
        std::shared_ptr<CallStatement> callStmt(new CallStatement);
        callStmt->setDest(callDest);
        callStmt->setIsComputed(false);

        rtl->append(std::make_shared<Assign>(SizeType::get(32), Location::regOf(REG_PPC_LR),
                                             Const::get(insn.m_addr + PPC_MAX_INSTRUCTION_LENGTH)));
        rtl->append(callStmt);

        if (m_prog) {
            Function *callee = m_prog->getOrCreateFunction(callDest);
            if (callee && callee != reinterpret_cast<Function *>(-1)) {
                callStmt->setDestProc(callee);
            }
        }
    }
    else if (insnID == "BCTR") {
        std::shared_ptr<CaseStatement> jump(new CaseStatement);
        jump->setDest(Location::regOf(REG_PPC_CTR));
        jump->setIsComputed(true);
        rtl->append(jump);
    }
    else if (insnID == "BCTRL") {
        rtl->append(std::make_shared<Assign>(SizeType::get(32), Location::regOf(REG_PPC_LR),
                                             Const::get(Address(insn.m_addr + 4))));

        std::shared_ptr<CallStatement> call(new CallStatement);
        call->setDest(Location::regOf(REG_PPC_CTR));
        call->setIsComputed(true);
        rtl->append(call);
    }
    else if (insnID == "BGT") {
        std::shared_ptr<BranchStatement> jump = rtl->back()->as<BranchStatement>();
        jump->setCondType(BranchType::JSG);
        if (numOperands == 0 || !insn.m_operands[numOperands - 1]->isIntConst()) {
            jump->setDest(insn.m_addr);
        }
        else {
            jump->setDest(insn.m_operands[numOperands - 1]);
        }
    }
    else if (insnID == "BGE") {
        std::shared_ptr<BranchStatement> jump = rtl->back()->as<BranchStatement>();
        jump->setCondType(BranchType::JSGE);
        if (numOperands == 0 || !insn.m_operands[numOperands - 1]->isIntConst()) {
            jump->setDest(insn.m_addr);
        }
        else {
            jump->setDest(insn.m_operands[numOperands - 1]);
        }
    }
    else if (insnID == "BLT") {
        std::shared_ptr<BranchStatement> jump = rtl->back()->as<BranchStatement>();
        jump->setCondType(BranchType::JSL);
        if (numOperands == 0 || !insn.m_operands[numOperands - 1]->isIntConst()) {
            jump->setDest(insn.m_addr);
        }
        else {
            jump->setDest(insn.m_operands[numOperands - 1]);
        }
    }
    else if (insnID == "BLE") {
        std::shared_ptr<BranchStatement> jump = rtl->back()->as<BranchStatement>();
        jump->setCondType(BranchType::JSLE);
        if (numOperands == 0 || !insn.m_operands[numOperands - 1]->isIntConst()) {
            jump->setDest(insn.m_addr);
        }
        else {
            jump->setDest(insn.m_operands[numOperands - 1]);
        }
    }
    else if (insnID == "BNE") {
        std::shared_ptr<BranchStatement> jump = rtl->back()->as<BranchStatement>();
        jump->setCondType(BranchType::JNE);
        if (numOperands == 0 || !insn.m_operands[numOperands - 1]->isIntConst()) {
            jump->setDest(insn.m_addr);
        }
        else {
            jump->setDest(insn.m_operands[numOperands - 1]);
        }
    }
    else if (insnID == "BEQ") {
        std::shared_ptr<BranchStatement> jump = rtl->back()->as<BranchStatement>();
        jump->setCondType(BranchType::JE);
        if (numOperands == 0 || !insn.m_operands[numOperands - 1]->isIntConst()) {
            jump->setDest(insn.m_addr);
        }
        else {
            jump->setDest(insn.m_operands[numOperands - 1]);
        }
    }
    else if (insnID == "BDNZ" || insnID == "BDNZL") {
        const Address dest = insn.m_operands[numOperands - 1]->access<Const>()->getAddr();
        if (dest != insn.m_addr + PPC_MAX_INSTRUCTION_LENGTH) {
            std::shared_ptr<BranchStatement> jump(new BranchStatement);
            jump->setDest(dest);
            jump->setCondType(BranchType::JNE);
            rtl->append(jump);
        }
    }
    else if (insnID == "STMW") {
        rtl->clear();
        const RegNum startRegNum          = insn.m_operands[0]->access<Const, 1>()->getInt();
        const SharedConstExp startAddrExp = Unary::get(opAddrOf, insn.m_operands[1]->clone())
                                                ->simplify();

        for (RegNum reg = startRegNum; reg <= REG_PPC_G31; ++reg) {
            const int i            = reg - startRegNum;
            const SharedExp memExp = Location::memOf(
                Binary::get(opPlus, startAddrExp->clone(), Const::get(4 * i)));

            std::shared_ptr<Assign> asgn(
                new Assign(SizeType::get(STD_SIZE), memExp->simplify(), Location::regOf(reg)));

            rtl->append(asgn);
        }
    }
    else if (insnID == "LMW") {
        rtl->clear();
        const RegNum startRegNum          = insn.m_operands[0]->access<Const, 1>()->getInt();
        const SharedConstExp startAddrExp = Unary::get(opAddrOf, insn.m_operands[1]->clone())
                                                ->simplify();

        for (RegNum reg = startRegNum; reg <= REG_PPC_G31; ++reg) {
            const int i            = reg - startRegNum;
            const SharedExp memExp = Location::memOf(
                Binary::get(opPlus, startAddrExp->clone(), Const::get(4 * i)));

            std::shared_ptr<Assign> asgn(
                new Assign(SizeType::get(STD_SIZE), Location::regOf(reg), memExp->simplify()));

            rtl->append(asgn);
        }
    }
    else if (insnID == "LBZU" || insnID == "LHZU" || insnID == "LWZU" || insnID == "LFSU" ||
             insnID == "LFDU" || insnID == "LHAU" || insnID == "STFSU" || insnID == "STFDU") {
        const QString msg = insn.m_operands[1]->toString();
        LOG_MSG("%1", msg);

        const SharedExp srcBase = Location::regOf(
            insn.m_operands[1]->access<Const, 1, 1, 1>()->getInt());
        const SharedExp offset = Const::get(insn.m_operands[1]->access<Const, 1, 2>()->getInt());

        rtl->append(std::make_shared<Assign>(SizeType::get(32), srcBase,
                                             Binary::get(opPlus, srcBase, offset)));
    }

    return rtl;
}


std::unique_ptr<RTL> CapstonePPCDecoder::instantiateRTL(const MachineInstruction &insn)
{
    if (m_debugMode) {
        QString argNames;
        for (std::size_t i = 0; i < insn.getNumOperands(); i++) {
            if (i != 0) {
                argNames += " ";
            }
            argNames += insn.m_operands[i]->toString();
        }

        LOG_MSG("Instantiating RTL at %1: %2 %3", insn.m_addr, insn.m_templateName, argNames);
    }

    // Take the argument, convert it to upper case and remove any .'s
    const QString sanitizedName = QString(insn.m_templateName).remove(".").toUpper();
    return m_dict.instantiateRTL(sanitizedName, insn.m_addr, insn.m_operands);
}


bool CapstonePPCDecoder::isCRManip(const cs::cs_insn *instruction) const
{
    switch (instruction->id) {
    case cs::PPC_INS_CRAND:
    case cs::PPC_INS_CRANDC:
    case cs::PPC_INS_CRCLR:
    case cs::PPC_INS_CREQV:
    case cs::PPC_INS_CRMOVE:
    case cs::PPC_INS_CRNAND:
    case cs::PPC_INS_CRNOR:
    case cs::PPC_INS_CRNOT:
    case cs::PPC_INS_CROR:
    case cs::PPC_INS_CRORC:
    case cs::PPC_INS_CRSET:
    case cs::PPC_INS_CRXOR: return true;
    }

    return false;
}


QString CapstonePPCDecoder::getTemplateName(const cs::cs_insn *instruction) const
{
    QString insnID = instruction->mnemonic; // cs::cs_insn_name(m_handle, instruction->id);
    insnID         = insnID.toUpper();

    // Chop off branch prediction hints
    if (insnID.endsWith("+") || insnID.endsWith("-")) {
        insnID = insnID.left(insnID.length() - 1);
    }

    // . cannot be part of an identifier -> use q instead
    insnID = insnID.replace('.', 'q');
    return insnID;
}


BOOMERANG_DEFINE_PLUGIN(PluginType::Decoder, CapstonePPCDecoder, "Capstone PPC decoder plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
