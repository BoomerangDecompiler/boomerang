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

    //     printf("%lx %08x %s %s\n", decodedInstruction->address, *(uint32 *)instructionData,
    //            decodedInstruction->mnemonic, decodedInstruction->op_str);

    result.type         = ICLASS::NOP; // only relevant for architectures with delay slots
    result.numBytes     = PPC_MAX_INSTRUCTION_LENGTH;
    result.reDecode     = false;
    result.rtl          = createRTLForInstruction(pc, decodedInstruction);
    result.valid        = (result.rtl != nullptr);

    cs_free(decodedInstruction, numInstructions);
    return true;
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
        return Location::memOf(Binary::get(opPlus, Location::regOf(fixRegNum(operand.mem.base)),
                                           Const::get(operand.mem.disp)))
            ->simplifyArith();
    }
    default: LOG_ERROR("Unknown ppc instruction operand type %1", (int)operand.type); break;
    }

    return nullptr;
}


std::unique_ptr<RTL> CapstonePPCDecoder::createRTLForInstruction(Address pc,
                                                                 cs::cs_insn *instruction)
{
    const int numOperands   = instruction->detail->ppc.op_count;
    cs::cs_ppc_op *operands = instruction->detail->ppc.operands;

    QString insnID = instruction->mnemonic; // cs::cs_insn_name(m_handle, instruction->id);
    insnID         = insnID.toUpper();

    // Chop off branch prediction hints
    if (insnID.endsWith("+") || insnID.endsWith("-")) {
        insnID = insnID.left(insnID.length() - 1);
    }

    // . cannot be part of an identifier -> use q instead
    insnID = insnID.replace('.', 'q');

    // Adjust the operands of cr* instructions (e.g. crxor).
    // This is to work around a bug in Capstone: The operands are disassembled as PPC_OP_REG
    // instead of PPC_OP_IMM or PPC_OP_CRX. See https://github.com/aquynh/capstone/issues/971
    // for details.
    if (isCRManip(instruction)) {
        for (int i = 0; i < numOperands; ++i) {
            const int bitNum = operands[i].reg - cs::PPC_REG_R0;
            operands[i].type = cs::PPC_OP_IMM;
            operands[i].imm  = bitNum;
        }
    }

    std::unique_ptr<RTL> rtl = instantiateRTL(pc, qPrintable(insnID), numOperands, operands);

    if (rtl == nullptr) {
        LOG_ERROR("Cannot find semantics for instruction '%1' at address %2, "
                  "treating instruction as NOP",
                  insnID, pc);
        return std::make_unique<RTL>(pc);
    }

    if (insnID == "BL" || insnID == "BLA") {
        Address callDest        = Address(operands[0].imm);
        CallStatement *callStmt = new CallStatement();
        callStmt->setDest(callDest);
        callStmt->setIsComputed(false);

        rtl->append(new Assign(SizeType::get(32), Location::regOf(REG_PPC_LR),
                               Const::get(pc + PPC_MAX_INSTRUCTION_LENGTH)));
        rtl->append(callStmt);

        if (m_prog) {
            Function *callee = m_prog->getOrCreateFunction(callDest);
            if (callee && callee != reinterpret_cast<Function *>(-1)) {
                callStmt->setDestProc(callee);
            }
        }
    }
    else if (insnID == "BCTR") {
        CaseStatement *jump = new CaseStatement();
        jump->setDest(Location::regOf(REG_PPC_CTR));
        jump->setIsComputed(true);
        rtl->append(jump);
    }
    else if (insnID == "BCTRL") {
        rtl->append(new Assign(SizeType::get(32), Location::regOf(REG_PPC_LR),
                               Const::get(Address(pc + 4))));

        CallStatement *call = new CallStatement();
        call->setDest(Location::regOf(REG_PPC_CTR));
        call->setIsComputed(true);
        rtl->append(call);
    }
    else if (insnID == "BGT") {
        BranchStatement *jump = new BranchStatement();
        if (numOperands == 0 || operands[numOperands - 1].type != cs::PPC_OP_IMM) {
            jump->setDest(pc);
        }
        else {
            jump->setDest(operandToExp(operands[numOperands - 1]));
        }
        jump->setCondType(BranchType::JSG);
        rtl->append(jump);
    }
    else if (insnID == "BGE") {
        BranchStatement *jump = new BranchStatement();
        if (numOperands == 0 || operands[numOperands - 1].type != cs::PPC_OP_IMM) {
            jump->setDest(pc);
        }
        else {
            jump->setDest(operandToExp(operands[numOperands - 1]));
        }
        jump->setCondType(BranchType::JSGE);
        rtl->append(jump);
    }
    else if (insnID == "BLT") {
        BranchStatement *jump = new BranchStatement();
        if (numOperands == 0 || operands[numOperands - 1].type != cs::PPC_OP_IMM) {
            jump->setDest(pc);
        }
        else {
            jump->setDest(operandToExp(operands[numOperands - 1]));
        }
        jump->setCondType(BranchType::JSL);
        rtl->append(jump);
    }
    else if (insnID == "BLE") {
        BranchStatement *jump = new BranchStatement();
        if (numOperands == 0 || operands[numOperands - 1].type != cs::PPC_OP_IMM) {
            jump->setDest(pc);
        }
        else {
            jump->setDest(operandToExp(operands[numOperands - 1]));
        }
        jump->setCondType(BranchType::JSLE);
        rtl->append(jump);
    }
    else if (insnID == "BNE") {
        BranchStatement *jump = new BranchStatement();
        if (numOperands == 0 || operands[numOperands - 1].type != cs::PPC_OP_IMM) {
            jump->setDest(pc);
        }
        else {
            jump->setDest(operandToExp(operands[numOperands - 1]));
        }
        jump->setCondType(BranchType::JNE);
        rtl->append(jump);
    }
    else if (insnID == "BEQ") {
        BranchStatement *jump = new BranchStatement();
        if (numOperands == 0 || operands[numOperands - 1].type != cs::PPC_OP_IMM) {
            jump->setDest(pc);
        }
        else {
            jump->setDest(operandToExp(operands[numOperands - 1]));
        }
        jump->setCondType(BranchType::JE);
        rtl->append(jump);
    }
    else if (insnID == "BDNZ" || insnID == "BDNZL") {
        const Address dest = operandToExp(operands[numOperands - 1])->access<Const>()->getAddr();
        if (dest != pc + PPC_MAX_INSTRUCTION_LENGTH) {
            BranchStatement *jump = new BranchStatement();
            jump->setDest(dest);
            jump->setCondType(BranchType::JNE);
            rtl->append(jump);
        }
    }
    else if (insnID == "STMW") {
        rtl->clear();
        const RegNum startRegNum          = fixRegNum(operands[0].reg);
        const SharedConstExp startAddrExp = Unary::get(opAddrOf, operandToExp(operands[1]))
                                                ->simplify();

        for (RegNum reg = startRegNum; reg <= REG_PPC_G31; ++reg) {
            const int i            = reg - startRegNum;
            const SharedExp memExp = Location::memOf(
                Binary::get(opPlus, startAddrExp->clone(), Const::get(4 * i)));

            Assign *asgn = new Assign(SizeType::get(STD_SIZE), memExp->simplify(),
                                      Location::regOf(reg));

            rtl->append(asgn);
        }
    }
    else if (insnID == "LMW") {
        rtl->clear();
        const RegNum startRegNum          = fixRegNum(operands[0].reg);
        const SharedConstExp startAddrExp = Unary::get(opAddrOf, operandToExp(operands[1]))
                                                ->simplify();

        for (RegNum reg = startRegNum; reg <= REG_PPC_G31; ++reg) {
            const int i            = reg - startRegNum;
            const SharedExp memExp = Location::memOf(
                Binary::get(opPlus, startAddrExp->clone(), Const::get(4 * i)));

            Assign *asgn = new Assign(SizeType::get(STD_SIZE), Location::regOf(reg),
                                      memExp->simplify());

            rtl->append(asgn);
        }
    }
    else if (insnID == "LBZU" || insnID == "LHZU" || insnID == "LWZU" || insnID == "LFSU" ||
             insnID == "LFDU" || insnID == "LHAU" || insnID == "STFSU" || insnID == "STFDU") {
        const SharedExp srcBase = Location::regOf(fixRegNum(operands[1].mem.base));
        const SharedExp offset  = Const::get(operands[1].mem.disp);

        rtl->append(new Assign(SizeType::get(32), srcBase, Binary::get(opPlus, srcBase, offset)));
    }

    return rtl;
}


std::unique_ptr<RTL> CapstonePPCDecoder::instantiateRTL(Address pc, const char *instructionID,
                                                        int numOperands,
                                                        const cs::cs_ppc_op *operands)
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

    // Take the argument, convert it to upper case and remove any .'s
    const QString sanitizedName = QString(instructionID).remove(".").toUpper();
    return m_dict.instantiateRTL(sanitizedName, pc, args);
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


BOOMERANG_DEFINE_PLUGIN(PluginType::Decoder, CapstonePPCDecoder, "Capstone PPC decoder plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
