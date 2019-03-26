#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CapstoneSPARCDecoder.h"

#include "boomerang/core/plugin/Plugin.h"
#include "boomerang/db/Prog.h"
#include "boomerang/ssl/statements/GotoStatement.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/util/log/Log.h"


#define SPARC_INSTRUCTION_LENGTH (4)


// only map those registers that are mapped to a number
// different from -1 in the SSL file.
// not all registers supported by capstone
// clang-format off
static std::map<cs::sparc_reg, RegNum> oldRegMap = {
    { cs::SPARC_REG_Y, REG_SPARC_Y },
    { cs::SPARC_REG_SP, REG_SPARC_SP }
};
// clang-format on


/**
 * Translates Capstone register IDs to Boomerang internal register IDs.
 * \returns RegNumSpecial if register not found.
 */
RegNum CapstoneSPARCDecoder::fixRegNum(const cs::cs_insn *insn, int opIdx) const
{
    assert(insn != nullptr);
    assert(opIdx < insn->detail->sparc.op_count);
    assert(insn->detail->sparc.operands[opIdx].type == cs::SPARC_OP_REG);

    const int csRegID = insn->detail->sparc.operands[opIdx].reg;

    if (csRegID >= cs::SPARC_REG_F0 && csRegID <= cs::SPARC_REG_F31) {
        if (getRegOperandSize(insn, opIdx) == 64) {
            return REG_SPARC_F0TO1 + (csRegID - cs::SPARC_REG_F0) / 2;
        }
        else if (getRegOperandSize(insn, opIdx) == 128) {
            return REG_SPARC_F0TO3 + (csRegID - cs::SPARC_REG_F0) / 4;
        }
        else { // single float
            return REG_SPARC_F0 + (csRegID - cs::SPARC_REG_F0);
        }
    }

    return fixRegNum(csRegID);
}


RegNum CapstoneSPARCDecoder::fixRegNum(int csRegID) const
{
    if (csRegID >= cs::SPARC_REG_G0 && csRegID <= cs::SPARC_REG_G7) {
        return REG_SPARC_G0 + (csRegID - cs::SPARC_REG_G0);
    }
    else if (csRegID >= cs::SPARC_REG_O0 && csRegID <= cs::SPARC_REG_O7) {
        return REG_SPARC_O0 + (csRegID - cs::SPARC_REG_O0);
    }
    else if (csRegID >= cs::SPARC_REG_I0 && csRegID <= cs::SPARC_REG_I7) {
        return REG_SPARC_I0 + (csRegID - cs::SPARC_REG_I0);
    }
    else if (csRegID >= cs::SPARC_REG_L0 && csRegID <= cs::SPARC_REG_L7) {
        return REG_SPARC_L0 + (csRegID - cs::SPARC_REG_L0);
    }
    else if (csRegID >= cs::SPARC_REG_F0 && csRegID <= cs::SPARC_REG_F31) {
        return REG_SPARC_F0 + (csRegID - cs::SPARC_REG_F0);
    }
    else if (csRegID >= cs::SPARC_REG_F32 && csRegID <= cs::SPARC_REG_F62) {
        return REG_SPARC_F0TO1 + (csRegID - cs::SPARC_REG_F32);
    }

    auto it = oldRegMap.find((cs::sparc_reg)csRegID);
    return (it != oldRegMap.end()) ? it->second : RegNumSpecial;
}



SharedExp CapstoneSPARCDecoder::getRegExp(const cs::cs_insn *insn, int opIdx) const
{
    assert(insn != nullptr);
    assert(opIdx < insn->detail->sparc.op_count);
    assert(insn->detail->sparc.operands[opIdx].type == cs::SPARC_OP_REG);
    const int csRegID = insn->detail->sparc.operands[opIdx].reg;

    if (csRegID == cs::SPARC_REG_G0) {
        return Const::get(0);
    }
    else {
        return Location::regOf(fixRegNum(insn, opIdx));
    }
}


SharedExp CapstoneSPARCDecoder::getRegExp(int csRegID) const
{
    if (csRegID == cs::SPARC_REG_G0) {
        return Const::get(0);
    }
    else {
        return Location::regOf(fixRegNum(csRegID));
    }
}



CapstoneSPARCDecoder::CapstoneSPARCDecoder(Project *project)
    : CapstoneDecoder(project, cs::CS_ARCH_SPARC,
                      (cs::cs_mode)(cs::CS_MODE_BIG_ENDIAN), "ssl/sparc.ssl")
{
}


bool CapstoneSPARCDecoder::decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result)
{
    const Byte *instructionData = reinterpret_cast<const Byte *>((HostAddress(delta) + pc).value());

    cs::cs_insn *decodedInstruction;
    size_t numInstructions = cs_disasm(m_handle, instructionData, SPARC_INSTRUCTION_LENGTH,
                                       pc.value(), 1, &decodedInstruction);

    result.valid = numInstructions > 0;
    if (!result.valid) {
        return false;
    }

//     printf("0x%lx %08x %s %s\n", decodedInstruction->address, *(uint32 *)instructionData,
//            decodedInstruction->mnemonic, decodedInstruction->op_str);

    result.type         = getInstructionType(decodedInstruction);
    result.numBytes     = SPARC_INSTRUCTION_LENGTH;
    result.reDecode     = false;
    result.rtl          = createRTLForInstruction(pc, decodedInstruction);
    result.forceOutEdge = Address::ZERO;
    result.valid        = (result.rtl != nullptr);

    cs_free(decodedInstruction, numInstructions);
    return true;
}


RegNum CapstoneSPARCDecoder::getRegNumByName(const QString &name) const
{
    // todo: slow
    for (size_t i = cs::SPARC_REG_F0; i < cs::SPARC_REG_ENDING; i++) {
        if (name == cs::cs_reg_name(m_handle, i)) {
            return fixRegNum(nullptr, i);
        }
    }

    return cs::SPARC_REG_INVALID;
}


QString CapstoneSPARCDecoder::getRegNameByNum(RegNum regNum) const
{
    return m_dict.getRegDB()->getRegNameByNum(regNum);
}


int CapstoneSPARCDecoder::getRegSizeByNum(RegNum regNum) const
{
    return m_dict.getRegDB()->getRegSizeByNum(regNum);
}


SharedExp CapstoneSPARCDecoder::operandToExp(const cs::cs_insn *instruction, int opIdx) const
{
    const cs::cs_sparc_op &operand = instruction->detail->sparc.operands[opIdx];
    switch (operand.type) {
    case cs::SPARC_OP_IMM: {
        return Const::get(Address(operand.imm));
    }
    case cs::SPARC_OP_REG: {
        return getRegExp(instruction, opIdx);
    }
    case cs::SPARC_OP_MEM: {
        SharedExp memExp = getRegExp(operand.mem.base);

        if (operand.mem.index != cs::SPARC_REG_INVALID) {
            memExp = Binary::get(opPlus, memExp, getRegExp(operand.mem.index));
        }

        memExp = Binary::get(opPlus, memExp, Const::get(operand.mem.disp));
        return Location::memOf(memExp)->simplifyArith();
    }
    default: LOG_ERROR("Unknown sparc instruction operand type %1", operand.type); break;
    }

    return nullptr;
}


std::unique_ptr<RTL> CapstoneSPARCDecoder::createRTLForInstruction(Address pc,
                                                                    cs::cs_insn* instruction)
{
    cs::cs_sparc_op *operands = instruction->detail->sparc.operands;

    QString insnID = instruction->mnemonic; // cs::cs_insn_name(m_handle, instruction->id);
    insnID         = insnID.remove(',').toUpper();

    if (instruction->id == cs::SPARC_INS_LDD) {
        const bool isFloatReg = instruction->detail->sparc.operands[1].reg >= cs::SPARC_REG_F0 &&
            instruction->detail->sparc.operands[1].reg <= cs::SPARC_REG_F62;

        if (isFloatReg) {
            insnID = "LDDF";
        }
    }
    else if (instruction->id == cs::SPARC_INS_STD) {
        const bool isFloatReg = instruction->detail->sparc.operands[0].reg >= cs::SPARC_REG_F0 &&
        instruction->detail->sparc.operands[0].reg <= cs::SPARC_REG_F62;

        if (isFloatReg) {
            insnID = "STDF";
        }
    }

    std::unique_ptr<RTL> rtl = instantiateRTL(pc, qPrintable(insnID), instruction);

    if (insnID == "BA" || insnID == "BAA" || insnID == "BN" || insnID == "BNA") {
        rtl->clear();
        rtl->append(new GotoStatement(Address(operands[0].imm)));
    }
    else if (insnID == "FBA" || insnID == "FBAA" || insnID == "FBN" || insnID == "FBNA") {
        rtl->clear();
        rtl->append(new GotoStatement(Address(operands[0].imm)));
    }
    else if (instruction->id == cs::SPARC_INS_B) {
        rtl->clear();
        BranchStatement *branch = new BranchStatement;
        branch->setDest(Address(operands[0].imm));
        branch->setIsComputed(false);

        BranchType bt = BranchType::INVALID;

        switch (instruction->detail->sparc.cc) {
            case cs::SPARC_CC_ICC_NE: bt = BranchType::JNE; break;
            case cs::SPARC_CC_ICC_E:  bt = BranchType::JE;  break;
            case cs::SPARC_CC_ICC_G:  bt = BranchType::JSG; break;
            case cs::SPARC_CC_ICC_LE: bt = BranchType::JSLE; break;
            case cs::SPARC_CC_ICC_GE: bt = BranchType::JSGE; break;
            case cs::SPARC_CC_ICC_L:  bt = BranchType::JSL;  break;
            case cs::SPARC_CC_ICC_GU: bt = BranchType::JUG; break;
            case cs::SPARC_CC_ICC_LEU: bt = BranchType::JULE; break;
            case cs::SPARC_CC_ICC_CC:  bt = BranchType::JUGE; break;
            case cs::SPARC_CC_ICC_CS:  bt = BranchType::JUL; break;
            case cs::SPARC_CC_ICC_POS: bt = BranchType::JPOS; break;
            case cs::SPARC_CC_ICC_NEG: bt = BranchType::JMI;  break;
            default: break;
        }

        branch->setCondType(bt);
        rtl->append(branch);
    }
    else if (instruction->id == cs::SPARC_INS_FB) {
        rtl->clear();
        BranchStatement *branch = new BranchStatement;
        branch->setDest(Address(operands[0].imm));
        branch->setIsComputed(false);

        BranchType bt = BranchType::INVALID;

        switch (instruction->detail->sparc.cc) {
            case cs::SPARC_CC_FCC_NE: bt = BranchType::JNE;  break;
            case cs::SPARC_CC_FCC_E:  bt = BranchType::JE;   break;
            case cs::SPARC_CC_FCC_G:  bt = BranchType::JSG;  break;
            case cs::SPARC_CC_FCC_LE: bt = BranchType::JSLE; break;
            case cs::SPARC_CC_FCC_GE: bt = BranchType::JSGE; break;
            case cs::SPARC_CC_FCC_L:  bt = BranchType::JSL;  break;
            case cs::SPARC_CC_FCC_UG: bt = BranchType::JSG;  break;
            case cs::SPARC_CC_FCC_UL: bt = BranchType::JSL;  break;
            case cs::SPARC_CC_FCC_LG: bt = BranchType::JNE;  break;
            case cs::SPARC_CC_FCC_UE: bt = BranchType::JE;   break;
            case cs::SPARC_CC_FCC_UGE: bt = BranchType::JSGE; break;
            case cs::SPARC_CC_FCC_ULE: bt = BranchType::JSLE; break;
            default: break;
        }

        branch->setCondType(bt, true);
        rtl->append(branch);
    }
    else if (instruction->id == cs::SPARC_INS_CALL) {
        const Address callDest = Address(operands[0].imm);

        rtl->clear();
        CallStatement *call = new CallStatement;
        call->setIsComputed(false);
        call->setDest(callDest);
        rtl->append(call);

        if (m_prog) {
            Function *destProc = m_prog->getOrCreateFunction(callDest);

            if (destProc == reinterpret_cast<Function *>(-1)) {
                destProc = nullptr;
            }

            call->setDestProc(destProc);
        }
    }
    else if (instruction->id == cs::SPARC_INS_JMPL) {
        rtl->clear();
        CaseStatement *caseStmt = new CaseStatement;
        caseStmt->setIsComputed(true);

        // Capstone returns the operand as SPARC_OP_MEM, so we have to "undo" the outermost memof
        // returned by operandToExp by an addrof
        caseStmt->setDest(Unary::get(opAddrOf, operandToExp(instruction, 0)));
        rtl->append(caseStmt);
    }


    if (rtl == nullptr) {
        LOG_ERROR("Encountered invalid or unknown instruction '%1 %2', treating instruction as NOP",
                  insnID, instruction->op_str);
        return std::make_unique<RTL>(pc);
    }

    return rtl;
}


std::unique_ptr<RTL> CapstoneSPARCDecoder::instantiateRTL(Address pc, const char *instructionID,
                                                          const cs::cs_insn *instruction)
{
    const int numOperands = instruction->detail->sparc.op_count;
    std::vector<SharedExp> args(numOperands);

    for (int i = 0; i < numOperands; i++) {
        args[i] = operandToExp(instruction, i);
    }

//     if (m_debugMode) {
        QString argNames;
        for (int i = 0; i < numOperands; i++) {
            if (i != 0) {
                argNames += " ";
            }
            argNames += args[i]->toString();
        }

        LOG_MSG("Instantiating RTL at %1: %2 %3", pc, instructionID, argNames);
//     }

    // Take the argument, convert it to upper case and remove any .'s
    const QString sanitizedName = QString(instructionID).remove(".").toUpper();
    return m_dict.instantiateRTL(sanitizedName, pc, args);
}


// clang-format off
static const std::map<QString, ICLASS> g_instructionTypes = {
    { "ba",     ICLASS::SD      },
    { "ba,a",   ICLASS::SU      },
    { "bn",     ICLASS::NCT     },
    { "bn,a",   ICLASS::SKIP    },
    { "bne",    ICLASS::SCD     },
    { "bne,a",  ICLASS::SCDAN   },
    { "be",     ICLASS::SCD     },
    { "be,a",   ICLASS::SCDAN   },
    { "bg",     ICLASS::SCD     },
    { "bg,a",   ICLASS::SCDAN   },
    { "ble",    ICLASS::SCD     },
    { "ble,a",  ICLASS::SCDAN   },
    { "bge",    ICLASS::SCD     },
    { "bge,a",  ICLASS::SCDAN   },
    { "bl",     ICLASS::SCD     },
    { "bl,a",   ICLASS::SCDAN   },
    { "bgu",    ICLASS::SCD     },
    { "bgu,a",  ICLASS::SCDAN   },
    { "bleu",   ICLASS::SCD     },
    { "bleu,a", ICLASS::SCDAN   },
    { "bcc",    ICLASS::SCD     },
    { "bcc,a",  ICLASS::SCDAN   },
    { "bcs",    ICLASS::SCD     },
    { "bcs,a",  ICLASS::SCDAN   },
    { "bge",    ICLASS::SCD     },
    { "bge,a",  ICLASS::SCDAN   },
    { "bpos",   ICLASS::SCD     },
    { "bpos,a", ICLASS::SCDAN   },
    { "bneg",   ICLASS::SCD     },
    { "bneg,a", ICLASS::SCDAN   },
    { "call",   ICLASS::SD      },
    { "fba",    ICLASS::SD      },
    { "fba,a",  ICLASS::SU      },
    { "fbn",    ICLASS::NCT     },
    { "fbn,a",  ICLASS::SKIP    },
    { "fbg",    ICLASS::SCD     },
    { "fbg,a",  ICLASS::SCDAN   },
    { "fbug",   ICLASS::SCD     },
    { "fbug,a", ICLASS::SCDAN   },
    { "fbl",    ICLASS::SCD     },
    { "fbl,a",  ICLASS::SCDAN   },
    { "fbul",   ICLASS::SCD     },
    { "fbul,a", ICLASS::SCDAN   },
    { "fblg",   ICLASS::SCD     },
    { "fblg,a", ICLASS::SCDAN   },
    { "fbne",   ICLASS::SCD     },
    { "fbne,a", ICLASS::SCDAN   },
    { "fbe",    ICLASS::SCD     },
    { "fbe,a",  ICLASS::SCDAN   },
    { "fbue",   ICLASS::SCD     },
    { "fbue,a", ICLASS::SCDAN   },
    { "fbge",   ICLASS::SCD     },
    { "fbge,a", ICLASS::SCDAN   },
    { "fbuge",  ICLASS::SCD     },
    { "fbuge,a",ICLASS::SCDAN   },
    { "fble",   ICLASS::SCD     },
    { "fble,a", ICLASS::SCDAN   },
    { "fbule",  ICLASS::SCD     },
    { "fbule,a",ICLASS::SCDAN   },

    { "jmpl",   ICLASS::DD      },
};
// clang-format on


ICLASS CapstoneSPARCDecoder::getInstructionType(const cs::cs_insn *instruction)
{
    if (instruction->id == cs::SPARC_INS_NOP) {
        return ICLASS::NOP;
    }

    // FIXME: This code should check instruction->detail.sparc instead, however Casptone
    // still has some bugs wrt. condition codes of branches, e.g. ba has cc invalid instead of 'a'
    const QString insMnemonic = QString(instruction->mnemonic);
    const auto it = g_instructionTypes.find(insMnemonic);
    return it != g_instructionTypes.end() ? it->second : ICLASS::NCT;
}


int CapstoneSPARCDecoder::getRegOperandSize(const cs::cs_insn* instruction, int opIdx) const
{
    switch (instruction->id) {
        // these always have 32 bit operands
    case cs::SPARC_INS_FSTOI:
    case cs::SPARC_INS_FITOS:
    case cs::SPARC_INS_FSQRTS:
        return 32;

        // these always have 64 bit operands
    case cs::SPARC_INS_FCMPD:
    case cs::SPARC_INS_FCMPED:
    case cs::SPARC_INS_FDIVD:
    case cs::SPARC_INS_FMULD:
    case cs::SPARC_INS_FSQRTD:
    case cs::SPARC_INS_FSUBD:
    case cs::SPARC_INS_LDD: // LDDF
    case cs::SPARC_INS_STD: // STDF
        return 64;

        // these always have 128 bit operands
    case cs::SPARC_INS_FCMPQ:
    case cs::SPARC_INS_FDIVQ:
    case cs::SPARC_INS_FMULQ:
    case cs::SPARC_INS_FSQRTQ:
    case cs::SPARC_INS_FSUBQ:
        return 128;

    case cs::SPARC_INS_FDTOI:
    case cs::SPARC_INS_FDTOS:
        return (opIdx == 0) ? 64 : 32;

    case cs::SPARC_INS_FQTOS:
    case cs::SPARC_INS_FQTOI:
        return (opIdx == 0) ? 128 : 32;

    case cs::SPARC_INS_FQTOD:
        return (opIdx == 0) ? 128 : 64;

    case cs::SPARC_INS_FDTOQ:
        return (opIdx == 0) ? 64 : 128;

    case cs::SPARC_INS_FITOD:
    case cs::SPARC_INS_FSTOD:
        return (opIdx == 0) ? 32 : 64;

    case cs::SPARC_INS_FITOQ:
    case cs::SPARC_INS_FSTOQ:
        return (opIdx == 0) ? 32 : 128;
    };


    return 32;
}


BOOMERANG_DEFINE_PLUGIN(PluginType::Decoder, CapstoneSPARCDecoder, "Capstone SPARC decoder plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
