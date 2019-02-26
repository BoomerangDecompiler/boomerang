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
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/util/log/Log.h"


#define PPC_MAX_INSTRUCTION_LENGTH (4)

// only map those registers that are mapped to a number
// different from -1 in the SSL file.
// not all registers supported by capstone
static std::map<cs::ppc_reg, RegNum> oldRegMap = {
};


/**
 * Translates Capstone register IDs to Boomerang internal register IDs.
 * \returns RegNumSpecial if register not found.
 */
RegNum fixRegNum(int csRegID)
{
    if (csRegID >= cs::PPC_REG_F0 && csRegID <= cs::PPC_REG_F31) {
        return 32 + (csRegID - cs::PPC_REG_F0);
    }
    else if (csRegID >= cs::PPC_REG_R0 && csRegID <= cs::PPC_REG_R31) {
        return 0 + (csRegID - cs::PPC_REG_R0);
    }
    else if (csRegID >= cs::PPC_REG_CR0 && csRegID <= cs::PPC_REG_CR7) {
        return 64 + (csRegID - cs::PPC_REG_CR0);
    }

    auto it = oldRegMap.find((cs::ppc_reg)csRegID);
    return (it != oldRegMap.end()) ? it->second : RegNumSpecial;
}


CapstonePPCDecoder::CapstonePPCDecoder(Project *project)
    : CapstoneDecoder(project, cs::CS_ARCH_PPC,
                      (cs::cs_mode)(cs::CS_MODE_32 + cs::CS_MODE_BIG_ENDIAN), "ssl/ppc.ssl")
{
}


bool CapstonePPCDecoder::decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result)
{
    const Byte *instructionData = reinterpret_cast<const Byte *>((HostAddress(delta) + pc).value());

    cs::cs_insn *decodedInstruction;
    size_t numInstructions = cs_disasm(m_handle, instructionData, PPC_MAX_INSTRUCTION_LENGTH,
                                       pc.value(), 1, &decodedInstruction);


    result.valid = numInstructions > 0;
    if (!result.valid) {
        return false;
    }

    printf("%lx %08x %s %s\n", decodedInstruction->address, *(uint32*)instructionData, decodedInstruction->mnemonic, decodedInstruction->op_str);

    result.type = ICLASS::NOP; // only relevant for architectures with delay slots
    result.numBytes = PPC_MAX_INSTRUCTION_LENGTH;
    result.reDecode = false;
    result.rtl = createRTLForInstruction(pc, decodedInstruction);
    result.forceOutEdge = Address::ZERO;
    result.valid = (result.rtl != nullptr);

    cs_free(decodedInstruction, numInstructions);
    return true;
}


RegNum CapstonePPCDecoder::getRegNumByName(const QString &name) const
{
    // todo: slow
    for (size_t i = cs::PPC_REG_CARRY; i < cs::PPC_REG_ENDING; i++) {
        if (name == cs::cs_reg_name(m_handle, i)) {
            return fixRegNum(i);
        }
    }

    return cs::PPC_REG_INVALID;
}


QString CapstonePPCDecoder::getRegNameByNum(RegNum regNum) const
{
    return m_dict.getRegDB()->getRegNameByNum(regNum);
}


int CapstonePPCDecoder::getRegSizeByNum(RegNum regNum) const
{
    return m_dict.getRegDB()->getRegSizeByNum(regNum);
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
            return Location::memOf(Binary::get(opPlus,
                                               Location::regOf(fixRegNum(operand.mem.base)),
                                               Const::get(operand.mem.disp)))->simplifyArith();
        }
        default: LOG_ERROR("Unknown ppc instruction operand type %1", operand.type); break;
    }

    return nullptr;
}


std::unique_ptr<RTL> CapstonePPCDecoder::createRTLForInstruction(Address pc, cs::cs_insn *instruction)
{
    const int numOperands         = instruction->detail->ppc.op_count;
    const cs::cs_ppc_op *operands = instruction->detail->ppc.operands;

    QString insnID = instruction->mnemonic; // cs::cs_insn_name(m_handle, instruction->id);
    insnID = insnID.toUpper();

    if (insnID == "CMPLWI" || insnID == "CMPWI" || insnID == "CMPW") {
        insnID = insnID + QString::number(numOperands);
    }
    else if (insnID.endsWith("+") || insnID.endsWith("-")) {
        insnID = insnID.left(insnID.length()-1);
    }

    std::unique_ptr<RTL> rtl = instantiateRTL(pc, qPrintable(insnID), numOperands, operands);

    if (rtl == nullptr) {
        LOG_ERROR("Encountered invalid or unknown instruction '%1 %2', treating instruction as NOP",
                  insnID, instruction->op_str);
        return std::make_unique<RTL>(pc);
    }

    if (insnID == "B") {
        GotoStatement *jump = new GotoStatement(Address(instruction->detail->ppc.operands[0].imm));
        jump->setIsComputed(false);
        rtl->append(jump);
    }
    else if (insnID == "BL") {
        Address callDest = Address(instruction->detail->ppc.operands[0].imm);
        CallStatement *callStmt = new CallStatement();
        callStmt->setDest(callDest);
        callStmt->setIsComputed(false);
        rtl->append(callStmt);

        Function *callee = m_prog->getOrCreateFunction(callDest);
        if (callee && callee != reinterpret_cast<Function *>(-1)) {
            callStmt->setDestProc(callee);
        }
    }
    else if (insnID == "BLR") {
        rtl->append(new ReturnStatement());
    }
    else if (insnID == "BGT") {
        BranchStatement *jump = new BranchStatement();
        jump->setDest(Address(instruction->detail->ppc.operands[0].imm));
        jump->setCondType(BranchType::JSG);
        rtl->append(jump);
    }
    else if (insnID == "BGE") {
        BranchStatement *jump = new BranchStatement();
        jump->setDest(Address(instruction->detail->ppc.operands[0].imm));
        jump->setCondType(BranchType::JSGE);
        rtl->append(jump);
    }
    else if (insnID == "BLT") {
        BranchStatement *jump = new BranchStatement();
        jump->setDest(Address(instruction->detail->ppc.operands[0].imm));
        jump->setCondType(BranchType::JSL);
        rtl->append(jump);
    }
    else if (insnID == "BLE") {
        BranchStatement *jump = new BranchStatement();
        jump->setDest(Address(instruction->detail->ppc.operands[0].imm));
        jump->setCondType(BranchType::JSLE);
        rtl->append(jump);
    }
    else if (insnID == "BNE") {
        BranchStatement *jump = new BranchStatement();
        jump->setDest(Address(instruction->detail->ppc.operands[0].imm));
        jump->setCondType(BranchType::JNE);
        rtl->append(jump);
    }
    else if (insnID == "BEQ") {
        BranchStatement *jump = new BranchStatement();
        jump->setDest(Address(instruction->detail->ppc.operands[0].imm));
        jump->setCondType(BranchType::JE);
        rtl->append(jump);
    }
    else if (insnID == "BDNZL") {
        const Address dest = Address(instruction->detail->ppc.operands[0].imm);
        if (dest != pc + PPC_MAX_INSTRUCTION_LENGTH) {
            BranchStatement *jump = new BranchStatement();
            jump->setDest(dest);
            jump->setCondType(BranchType::JNE);
            rtl->append(jump);
        }
    }
    else if (insnID == "CRCLR") {
        // This is because of a bug in Capstone: See https://github.com/aquynh/capstone/issues/971
        rtl->clear();

        // 4 == number of bits in a single condition register
        int bitNum = (instruction->detail->ppc.operands[0].reg - cs::PPC_REG_R0);
        const RegNum adjustedRegNum = (bitNum/4) + REG_PPC_CR0;
        bitNum %= 4;

        rtl->append(new Assign(SizeType::get(1),
                               Ternary::get(opAt,
                                            Location::regOf(adjustedRegNum),
                                            Const::get(bitNum),
                                            Const::get(bitNum)),
                               Const::get(0)));
    }
    else if (insnID == "STMW") {
        rtl->clear();
        const RegNum startRegNum = fixRegNum(operands[0].reg);
        const SharedConstExp startAddrExp = Unary::get(opAddrOf, operandToExp(operands[1]))->simplify();

        for (RegNum reg = startRegNum; reg <= REG_PPC_G31; ++reg) {
            const int i = reg - startRegNum;
            const SharedExp memExp = Location::memOf(Binary::get(opPlus,
                                                                 startAddrExp->clone(),
                                                                 Const::get(4*i)));

            Assign *asgn = new Assign(SizeType::get(STD_SIZE),
                                      memExp->simplify(),
                                      Location::regOf(reg));

            rtl->append(asgn);
        }
    }
    else if (insnID == "LMW") {
        rtl->clear();
        const RegNum startRegNum = fixRegNum(operands[0].reg);
        const SharedConstExp startAddrExp = Unary::get(opAddrOf, operandToExp(operands[1]))->simplify();

        for (RegNum reg = startRegNum; reg <= REG_PPC_G31; ++reg) {
            const int i = reg - startRegNum;
            const SharedExp memExp = Location::memOf(Binary::get(opPlus,
                                                                 startAddrExp->clone(),
                                                                 Const::get(4*i)));

            Assign *asgn = new Assign(SizeType::get(STD_SIZE),
                                      Location::regOf(reg),
                                      memExp->simplify());

            rtl->append(asgn);
        }
    }

    return rtl;
}


std::unique_ptr<RTL> CapstonePPCDecoder::instantiateRTL(Address pc, const char *instructionID,
                                                        int numOperands, const cs::cs_ppc_op *operands)
{
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

    bool found;
    const std::pair<QString, DWord> &signature = m_dict.getSignature(instructionID, &found);

    if (found) {
        return m_dict.instantiateRTL(signature.first, pc, args);
    }
    else {
        return nullptr;
    }
}


BOOMERANG_DEFINE_PLUGIN(PluginType::Decoder, CapstonePPCDecoder, "Capstone PPC decoder plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
