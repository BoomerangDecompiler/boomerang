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


#define SPARC_INSTRUCTION_LENGTH (4)


// only map those registers that are mapped to a number
// different from -1 in the SSL file.
// not all registers supported by capstone
// clang-format off
static std::map<cs::sparc_reg, RegNum> oldRegMap = {
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
        return Location::regOf(fixRegNum(operand.reg));
    }
    case cs::SPARC_OP_MEM: {
        return Location::memOf(Binary::get(opPlus, Location::regOf(fixRegNum(operand.mem.base)),
                                           Const::get(operand.mem.disp)))
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
    insnID         = insnID.toUpper();

    std::unique_ptr<RTL> rtl = instantiateRTL(pc, qPrintable(insnID), numOperands, operands);

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
    { "fbuge",   ICLASS::SCD     },
    { "fbuge,a", ICLASS::SCDAN   },
    { "fble",   ICLASS::SCD     },
    { "fble,a", ICLASS::SCDAN   },
    { "fbule",   ICLASS::SCD     },
    { "fbule,a", ICLASS::SCDAN   }
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
