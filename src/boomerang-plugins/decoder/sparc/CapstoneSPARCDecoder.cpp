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
#include "boomerang/util/log/Log.h"

#include "boomerang/ssl/statements/GotoStatement.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"


#define SPARC_INSTRUCTION_LENGTH (4)


// only map those registers that are mapped to a number
// different from -1 in the SSL file.
// not all registers supported by capstone
// clang-format off
static std::map<cs::sparc_reg, RegNum> oldRegMap = {
    { cs::SPARC_REG_Y, REG_SPARC_Y }
};
// clang-format on


/**
 * Translates Capstone register IDs to Boomerang internal register IDs.
 * \returns RegNumSpecial if register not found.
 */
RegNum fixRegNum(int csRegID)
{
    if (csRegID >= cs::SPARC_REG_F0 && csRegID <= cs::SPARC_REG_F31) {
        return REG_SPARC_F0 + (csRegID - cs::SPARC_REG_F0);
    }
    else if (csRegID >= cs::SPARC_REG_F32 && csRegID <= cs::SPARC_REG_F62) {
        return REG_SPARC_F0TO1 + (csRegID - cs::SPARC_REG_F32);
    }
    else if (csRegID >= cs::SPARC_REG_G0 && csRegID <= cs::SPARC_REG_G7) {
        return REG_SPARC_G0 + (csRegID - cs::SPARC_REG_G0);
    }
    else if (csRegID >= cs::SPARC_REG_O0 && csRegID <= cs::SPARC_REG_O7) {
        return REG_SPARC_O0 + (csRegID - cs::SPARC_REG_O0);
    }
    else if (csRegID >= cs::SPARC_REG_L0 && csRegID <= cs::SPARC_REG_L7) {
        return REG_SPARC_L0 + (csRegID - cs::SPARC_REG_L0);
    }

    auto it = oldRegMap.find((cs::sparc_reg)csRegID);
    return (it != oldRegMap.end()) ? it->second : RegNumSpecial;
}


SharedExp getRegExp(int csRegID)
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
            return fixRegNum(i);
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


SharedExp operandToExp(const cs::cs_sparc_op &operand)
{
    switch (operand.type) {
    case cs::SPARC_OP_IMM: {
        return Const::get(Address(operand.imm));
    }
    case cs::SPARC_OP_REG: {
        return getRegExp(operand.reg);
    }
    case cs::SPARC_OP_MEM: {
        SharedExp memExp = getRegExp(operand.mem.base);

        if (operand.mem.index != cs::SPARC_REG_INVALID) {
            memExp = Binary::get(opPlus, memExp, getRegExp(operand.mem.index));
        }

        return Location::memOf(Binary::get(opPlus, memExp, Const::get(operand.mem.disp)))
            ->simplifyArith();
    }
    default: LOG_ERROR("Unknown sparc instruction operand type %1", operand.type); break;
    }

    return nullptr;
}


std::unique_ptr<RTL> CapstoneSPARCDecoder::createRTLForInstruction(Address pc,
                                                                    cs::cs_insn* instruction)
{
    const int numOperands     = instruction->detail->sparc.op_count;
    cs::cs_sparc_op *operands = instruction->detail->sparc.operands;

    QString insnID = instruction->mnemonic; // cs::cs_insn_name(m_handle, instruction->id);
    insnID         = insnID.remove(',').toUpper();

    std::unique_ptr<RTL> rtl = instantiateRTL(pc, qPrintable(insnID), numOperands, operands);

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
        rtl->clear();
        CallStatement *call = new CallStatement;
        call->setIsComputed(false);
        call->setDest(Address(operands[0].imm));
        rtl->append(call);
    }
    else if (instruction->id == cs::SPARC_INS_JMPL) {
        rtl->clear();
        CaseStatement *caseStmt = new CaseStatement;
        caseStmt->setIsComputed(true);

        // Capstone returns the operand as SPARC_OP_MEM, so we have to "undo" the outermost memof
        // returned by operandToExp by an addrof
        caseStmt->setDest(Unary::get(opAddrOf, operandToExp(operands[0])));
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
                                                        int numOperands,
                                                        const cs::cs_sparc_op *operands)
{
    std::vector<SharedExp> args(numOperands);
    for (int i = 0; i < numOperands; i++) {
        args[i] = operandToExp(operands[i]);
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
    // FIXME: This code should check instruction->detail.sparc instead, however Casptone
    // still has some bugs wrt. condition codes of branches, e.g. ba has cc invalid instead of 'a'
    const QString insMnemonic = QString(instruction->mnemonic);
    const auto it = g_instructionTypes.find(insMnemonic);
    return it != g_instructionTypes.end() ? it->second : ICLASS::NCT;
}


BOOMERANG_DEFINE_PLUGIN(PluginType::Decoder, CapstoneSPARCDecoder, "Capstone SPARC decoder plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
