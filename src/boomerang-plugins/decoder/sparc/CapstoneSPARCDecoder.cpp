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
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/GotoStatement.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/util/log/Log.h"

#include <cstring>


#define SPARC_INSTRUCTION_LENGTH (4)


// only map those registers that are mapped to a number
// different from -1 in the SSL file.
// not all registers supported by capstone
// clang-format off
static std::map<cs::sparc_reg, RegNum> oldRegMap = {
    { cs::SPARC_REG_Y,   REG_SPARC_Y   },
    { cs::SPARC_REG_SP,  REG_SPARC_SP  },
    { cs::SPARC_REG_FP,  REG_SPARC_FP  },
    { cs::SPARC_REG_ICC, REG_SPARC_ICC },
    { cs::SPARC_REG_O6,  REG_SPARC_O6  },
    { cs::SPARC_REG_O7,  REG_SPARC_O7  }
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
    // Workaround for bug in Capstone (o0..o7 are not numbered sequentially).
    // o6 and o7 are handled by oldRegMap
    else if (csRegID >= cs::SPARC_REG_O0 && csRegID <= cs::SPARC_REG_O5) {
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
    : CapstoneDecoder(project, cs::CS_ARCH_SPARC, cs::CS_MODE_BIG_ENDIAN, "ssl/sparc.ssl")
{
}


bool CapstoneSPARCDecoder::decodeInstruction(Address pc, ptrdiff_t delta,
                                             MachineInstruction &result)
{
    const Byte *instructionData = reinterpret_cast<const Byte *>((HostAddress(delta) + pc).value());
    const Byte *oldInstructionData = instructionData;

    cs::cs_detail insnDetail;
    cs::cs_insn decodedInstruction;
    decodedInstruction.detail = &insnDetail;

    size_t bufsize   = SPARC_INSTRUCTION_LENGTH;
    uint64_t addr    = pc.value();
    const bool valid = cs::cs_disasm_iter(m_handle, &instructionData, &bufsize, &addr,
                                          &decodedInstruction);

    if (!valid) {
        // HACK: Capstone does not support ldd and std for gpr destinations,
        // so we have to test for it manually.
        const uint32_t insn = Util::readDWord(oldInstructionData, Endian::Big);

        if (!decodeLDD(&decodedInstruction, insn)) {
            if (!decodeSTD(&decodedInstruction, insn)) {
                return false;
            }
        }

        decodedInstruction.address = pc.value();
    }

    result.m_addr   = pc;
    result.m_id     = decodedInstruction.id;
    result.m_size   = decodedInstruction.size;
    result.m_iclass = getInstructionType(&decodedInstruction);

    std::strncpy(result.m_mnem.data(), decodedInstruction.mnemonic, MNEM_SIZE);
    std::strncpy(result.m_opstr.data(), decodedInstruction.op_str, OPSTR_SIZE);
    result.m_mnem[MNEM_SIZE - 1]   = '\0';
    result.m_opstr[OPSTR_SIZE - 1] = '\0';

    const std::size_t numOperands = decodedInstruction.detail->sparc.op_count;
    result.m_operands.resize(numOperands);

    for (std::size_t i = 0; i < numOperands; ++i) {
        result.m_operands[i] = operandToExp(&decodedInstruction, i);
    }

    result.m_templateName = getTemplateName(&decodedInstruction);
    result.m_sparcCC      = decodedInstruction.detail->sparc.cc;

    std::strncpy(result.m_mnem.data(), decodedInstruction.mnemonic, MNEM_SIZE);
    std::strncpy(result.m_opstr.data(), decodedInstruction.op_str, OPSTR_SIZE);
    result.m_mnem[MNEM_SIZE - 1]   = '\0';
    result.m_opstr[OPSTR_SIZE - 1] = '\0';

    return true;
}


bool CapstoneSPARCDecoder::liftInstruction(const MachineInstruction &insn, DecodeResult &lifted)
{
    lifted.iclass = insn.m_iclass;
    lifted.reLift = false;
    lifted.rtl    = createRTLForInstruction(insn);

    if (lifted.rtl && lifted.rtl->empty()) {
        // Force empty unrecognized instructions to have NOP type instead of NCT
        lifted.iclass = IClass::NOP;
    }

    return lifted.rtl != nullptr;
}


QString CapstoneSPARCDecoder::getRegNameByNum(RegNum regNum) const
{
    return m_dict.getRegDB()->getRegNameByNum(regNum);
}


int CapstoneSPARCDecoder::getRegSizeByNum(RegNum regNum) const
{
    return m_dict.getRegDB()->getRegSizeByNum(regNum);
}


bool CapstoneSPARCDecoder::isSPARCRestore(const MachineInstruction &insn) const
{
    return insn.m_id == cs::SPARC_INS_RESTORE;
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
    default: LOG_ERROR("Unknown sparc instruction operand type %1", (int)operand.type); break;
    }

    return nullptr;
}


std::unique_ptr<RTL> CapstoneSPARCDecoder::createRTLForInstruction(const MachineInstruction &insn)
{
    const std::size_t numOperands = insn.getNumOperands();

    std::unique_ptr<RTL> rtl = instantiateRTL(insn);
    const QString insnID     = insn.m_templateName;

    if (rtl == nullptr) {
        return nullptr;
    }


    if (insnID == "BA" || insnID == "BAA" || insnID == "BN" || insnID == "BNA") {}
    else if (insnID == "FBA" || insnID == "FBAA" || insnID == "FBN" || insnID == "FBNA") {
    }
    else if (insn.m_id == cs::SPARC_INS_B) {
        rtl->clear();
        std::shared_ptr<BranchStatement> branch(new BranchStatement);
        branch->setDest(Address(insn.m_operands[numOperands - 1]->access<Const>()->getLong()));
        branch->setIsComputed(false);

        BranchType bt = BranchType::INVALID;

        switch (insn.m_sparcCC) {
        case cs::SPARC_CC_ICC_NE: bt = BranchType::JNE; break;
        case cs::SPARC_CC_ICC_E: bt = BranchType::JE; break;
        case cs::SPARC_CC_ICC_G: bt = BranchType::JSG; break;
        case cs::SPARC_CC_ICC_LE: bt = BranchType::JSLE; break;
        case cs::SPARC_CC_ICC_GE: bt = BranchType::JSGE; break;
        case cs::SPARC_CC_ICC_L: bt = BranchType::JSL; break;
        case cs::SPARC_CC_ICC_GU: bt = BranchType::JUG; break;
        case cs::SPARC_CC_ICC_LEU: bt = BranchType::JULE; break;
        case cs::SPARC_CC_ICC_CC: bt = BranchType::JUGE; break;
        case cs::SPARC_CC_ICC_CS: bt = BranchType::JUL; break;
        case cs::SPARC_CC_ICC_POS: bt = BranchType::JPOS; break;
        case cs::SPARC_CC_ICC_NEG: bt = BranchType::JMI; break;
        default: assert(false);
        }

        branch->setCondType(bt);
        rtl->append(branch);
    }
    else if (insn.m_id == cs::SPARC_INS_FB) {
        rtl->clear();
        std::shared_ptr<BranchStatement> branch(new BranchStatement);
        branch->setDest(Address(insn.m_operands[0]->access<Const>()->getLong()));
        branch->setIsComputed(false);

        BranchType bt = BranchType::INVALID;

        switch (insn.m_sparcCC) {
        case cs::SPARC_CC_FCC_NE: bt = BranchType::JNE; break;
        case cs::SPARC_CC_FCC_E: bt = BranchType::JE; break;
        case cs::SPARC_CC_FCC_G: bt = BranchType::JSG; break;
        case cs::SPARC_CC_FCC_LE: bt = BranchType::JSLE; break;
        case cs::SPARC_CC_FCC_GE: bt = BranchType::JSGE; break;
        case cs::SPARC_CC_FCC_L: bt = BranchType::JSL; break;
        case cs::SPARC_CC_FCC_UG: bt = BranchType::JSG; break;
        case cs::SPARC_CC_FCC_UL: bt = BranchType::JSL; break;
        case cs::SPARC_CC_FCC_LG: bt = BranchType::JNE; break;
        case cs::SPARC_CC_FCC_UE: bt = BranchType::JE; break;
        case cs::SPARC_CC_FCC_UGE: bt = BranchType::JSGE; break;
        case cs::SPARC_CC_FCC_ULE: bt = BranchType::JSLE; break;
        default: break;
        }

        branch->setCondType(bt, true);
        rtl->append(branch);
    }
    else if (insn.m_id == cs::SPARC_INS_CALL) {
        rtl->clear();
        std::shared_ptr<CallStatement> call(new CallStatement);

        const bool isConstDest = insn.m_operands[0]->isConst();
        call->setIsComputed(!isConstDest);

        if (isConstDest) {
            const Address callDest = Address(insn.m_operands[0]->access<Const>()->getLong());
            call->setDest(callDest);

            if (m_prog) {
                Function *destProc = m_prog->getOrCreateFunction(callDest);

                if (destProc == reinterpret_cast<Function *>(-1)) {
                    destProc = nullptr;
                }

                call->setDestProc(destProc);
            }
        }
        else { // mem / reg
            SharedExp callDest = Unary::get(opAddrOf, insn.m_operands[0]->clone())->simplify();
            if (callDest->isConst()) {
                call->setIsComputed(false);
                call->setDest(callDest->access<Const>()->getAddr());

                if (m_prog) {
                    Function *destProc = m_prog->getOrCreateFunction(
                        callDest->access<Const>()->getAddr());

                    if (destProc == reinterpret_cast<Function *>(-1)) {
                        destProc = nullptr;
                    }

                    call->setDestProc(destProc);
                }
            }
            else {
                call->setIsComputed(true);
                call->setDest(callDest);
            }
        }

        rtl->append(call);
    }
    else if (insn.m_id == cs::SPARC_INS_JMPL || insn.m_id == cs::SPARC_INS_JMP) {
        rtl->clear();
        std::shared_ptr<CaseStatement> caseStmt(new CaseStatement);
        caseStmt->setIsComputed(true);

        // Capstone returns the operand as SPARC_OP_MEM, so we have to "undo" the outermost memof
        // returned by operandToExp by an addrof
        caseStmt->setDest(Unary::get(opAddrOf, insn.m_operands[0]->clone())->simplify());
        rtl->append(caseStmt);
    }

    return rtl;
}


std::unique_ptr<RTL> CapstoneSPARCDecoder::instantiateRTL(const MachineInstruction &insn)
{
    const int numOperands = insn.getNumOperands();

    if (m_debugMode) {
        QString argNames;
        for (int i = 0; i < numOperands; i++) {
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


// clang-format off
static const std::map<QString, IClass> g_instructionTypes = {
    { "ba",     IClass::SD      },
    { "ba,a",   IClass::SU      },
    { "bn",     IClass::NCT     },
    { "bn,a",   IClass::SKIP    },
    { "bne",    IClass::SCD     },
    { "bne,a",  IClass::SCDAN   },
    { "be",     IClass::SCD     },
    { "be,a",   IClass::SCDAN   },
    { "bg",     IClass::SCD     },
    { "bg,a",   IClass::SCDAN   },
    { "ble",    IClass::SCD     },
    { "ble,a",  IClass::SCDAN   },
    { "bge",    IClass::SCD     },
    { "bge,a",  IClass::SCDAN   },
    { "bl",     IClass::SCD     },
    { "bl,a",   IClass::SCDAN   },
    { "bgu",    IClass::SCD     },
    { "bgu,a",  IClass::SCDAN   },
    { "bleu",   IClass::SCD     },
    { "bleu,a", IClass::SCDAN   },
    { "bcc",    IClass::SCD     },
    { "bcc,a",  IClass::SCDAN   },
    { "bcs",    IClass::SCD     },
    { "bcs,a",  IClass::SCDAN   },
    { "bge",    IClass::SCD     },
    { "bge,a",  IClass::SCDAN   },
    { "bpos",   IClass::SCD     },
    { "bpos,a", IClass::SCDAN   },
    { "bneg",   IClass::SCD     },
    { "bneg,a", IClass::SCDAN   },
    { "call",   IClass::SD      },
    { "fba",    IClass::SD      },
    { "fba,a",  IClass::SU      },
    { "fbn",    IClass::NCT     },
    { "fbn,a",  IClass::SKIP    },
    { "fbg",    IClass::SCD     },
    { "fbg,a",  IClass::SCDAN   },
    { "fbug",   IClass::SCD     },
    { "fbug,a", IClass::SCDAN   },
    { "fbl",    IClass::SCD     },
    { "fbl,a",  IClass::SCDAN   },
    { "fbul",   IClass::SCD     },
    { "fbul,a", IClass::SCDAN   },
    { "fblg",   IClass::SCD     },
    { "fblg,a", IClass::SCDAN   },
    { "fbne",   IClass::SCD     },
    { "fbne,a", IClass::SCDAN   },
    { "fbe",    IClass::SCD     },
    { "fbe,a",  IClass::SCDAN   },
    { "fbue",   IClass::SCD     },
    { "fbue,a", IClass::SCDAN   },
    { "fbge",   IClass::SCD     },
    { "fbge,a", IClass::SCDAN   },
    { "fbuge",  IClass::SCD     },
    { "fbuge,a",IClass::SCDAN   },
    { "fble",   IClass::SCD     },
    { "fble,a", IClass::SCDAN   },
    { "fbule",  IClass::SCD     },
    { "fbule,a",IClass::SCDAN   },

    { "jmp",    IClass::DD      },
    { "jmpl",   IClass::DD      },
    { "ret",    IClass::DD      },
    { "retl",   IClass::DD      },
    { "rett",   IClass::DD      }
};
// clang-format on


IClass CapstoneSPARCDecoder::getInstructionType(const cs::cs_insn *instruction)
{
    if (instruction->id == cs::SPARC_INS_NOP) {
        return IClass::NOP;
    }
    else if (instruction->id == cs::SPARC_INS_UNIMP) {
        return IClass::NOP;
    }
    else if (instruction->id == cs::SPARC_INS_CALL &&
             instruction->detail->sparc.operands[0].type == cs::SPARC_OP_MEM) {
        if (instruction->detail->sparc.operands[0].mem.base == cs::SPARC_REG_G0) {
            return IClass::SD; // call %g0+foo, %o3. This is a static call
        }
        else {
            return IClass::DD; // computed call
        }
    }
    else if ((instruction->id == cs::SPARC_INS_JMP || instruction->id == cs::SPARC_INS_JMPL) &&
             instruction->detail->sparc.operands[0].type == cs::SPARC_OP_MEM) {
        if (instruction->detail->sparc.operands[0].mem.base == cs::SPARC_REG_G0) {
            return IClass::SD;
        }
        else {
            return IClass::DD;
        }
    }

    // FIXME: This code should check instruction->detail.sparc instead, however Casptone
    // still has some bugs wrt. condition codes of branches, e.g. ba has cc invalid instead of 'a'
    QString insMnemonic = QString(instruction->mnemonic);
    if (insMnemonic.endsWith(",pn") || insMnemonic.endsWith(",pt")) {
        insMnemonic.chop(3);
    }

    const auto it = g_instructionTypes.find(insMnemonic);
    return it != g_instructionTypes.end() ? it->second : IClass::NCT;
}


int CapstoneSPARCDecoder::getRegOperandSize(const cs::cs_insn *instruction, int opIdx) const
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
    case cs::SPARC_INS_FSUBQ: return 128;

    case cs::SPARC_INS_FDTOI:
    case cs::SPARC_INS_FDTOS: return (opIdx == 0) ? 64 : 32;

    case cs::SPARC_INS_FQTOS:
    case cs::SPARC_INS_FQTOI: return (opIdx == 0) ? 128 : 32;

    case cs::SPARC_INS_FQTOD: return (opIdx == 0) ? 128 : 64;

    case cs::SPARC_INS_FDTOQ: return (opIdx == 0) ? 64 : 128;

    case cs::SPARC_INS_FITOD:
    case cs::SPARC_INS_FSTOD: return (opIdx == 0) ? 32 : 64;

    case cs::SPARC_INS_FITOQ:
    case cs::SPARC_INS_FSTOQ: return (opIdx == 0) ? 32 : 128;
    };

    return 32;
}


/// Manual translation of register code to Capstone register
cs::sparc_reg fixSparcReg(uint8 code)
{
    if (code == 30) {
        return cs::SPARC_REG_FP;
    }
    else if (code == 14) {
        return cs::SPARC_REG_SP;
    }
    else if (code < 8) {
        return (cs::sparc_reg)(cs::SPARC_REG_G0 + (code & 7));
    }
    else if (code < 16) {
        return (cs::sparc_reg)(cs::SPARC_REG_O0 + (code & 7));
    }
    else if (code < 24) {
        return (cs::sparc_reg)(cs::SPARC_REG_L0 + (code & 7));
    }
    else {
        return (cs::sparc_reg)(cs::SPARC_REG_I0 + (code & 7));
    }
}


bool CapstoneSPARCDecoder::decodeLDD(cs::cs_insn *decodedInstruction, uint32_t insn) const
{
    if (((insn >> 19) & 0b1100000111111) != 0b1100000000011) {
        return false; // not ldd
    }

    const cs::sparc_reg rd  = fixSparcReg((insn >> 25) & 0x1F);
    const cs::sparc_reg rs1 = fixSparcReg((insn >> 14) & 0x1F);
    const bool hasImm       = ((insn >> 13) & 1) != 0;

    decodedInstruction->id   = cs::SPARC_INS_LDD;
    decodedInstruction->size = SPARC_INSTRUCTION_LENGTH;

    decodedInstruction->detail->sparc.cc       = cs::SPARC_CC_INVALID;
    decodedInstruction->detail->sparc.hint     = cs::SPARC_HINT_INVALID;
    decodedInstruction->detail->sparc.op_count = 2;

    decodedInstruction->detail->sparc.operands[0].type     = cs::SPARC_OP_MEM;
    decodedInstruction->detail->sparc.operands[0].mem.base = rs1;

    if (hasImm) {
        const int simm = Util::signExtend(insn & 0x1FFF, 13);
        decodedInstruction->detail->sparc.operands[0].mem.index = cs::SPARC_REG_INVALID;
        decodedInstruction->detail->sparc.operands[0].mem.disp  = simm;
        std::sprintf(decodedInstruction->op_str, "[%s + %d], %s", cs::cs_reg_name(m_handle, rs1),
                     simm, cs::cs_reg_name(m_handle, rd));
    }
    else { // reg offset
        const cs::sparc_reg rs2                                 = fixSparcReg(insn & 0x1F);
        decodedInstruction->detail->sparc.operands[0].mem.index = rs2;
        decodedInstruction->detail->sparc.operands[0].mem.disp  = 0;
        std::sprintf(decodedInstruction->op_str, "[%s + %s], %s", cs::cs_reg_name(m_handle, rs1),
                     cs::cs_reg_name(m_handle, rs2), cs::cs_reg_name(m_handle, rd));
    }

    decodedInstruction->detail->sparc.operands[1].type = cs::SPARC_OP_REG;
    decodedInstruction->detail->sparc.operands[1].reg  = rd;

    Util::writeDWord(&decodedInstruction->bytes, insn, Endian::Little);
    decodedInstruction->bytes[4] = 0;
    std::strcpy(decodedInstruction->mnemonic, "ldd");
    return true;
}


bool CapstoneSPARCDecoder::decodeSTD(cs::cs_insn *decodedInstruction, uint32_t insn) const
{
    if (((insn >> 19) & 0b1100000111111) != 0b1100000000111) {
        return false; // not std
    }

    const cs::sparc_reg rd  = fixSparcReg((insn >> 25) & 0x1F);
    const cs::sparc_reg rs1 = fixSparcReg((insn >> 14) & 0x1F);
    const bool hasImm       = ((insn >> 13) & 1) != 0;

    decodedInstruction->id   = cs::SPARC_INS_STD;
    decodedInstruction->size = SPARC_INSTRUCTION_LENGTH;

    decodedInstruction->detail->sparc.cc       = cs::SPARC_CC_INVALID;
    decodedInstruction->detail->sparc.hint     = cs::SPARC_HINT_INVALID;
    decodedInstruction->detail->sparc.op_count = 2;

    decodedInstruction->detail->sparc.operands[1].type     = cs::SPARC_OP_MEM;
    decodedInstruction->detail->sparc.operands[1].mem.base = rs1;

    if (hasImm) {
        const int simm = Util::signExtend(insn & 0x1FFF, 1);
        decodedInstruction->detail->sparc.operands[1].mem.index = cs::SPARC_REG_INVALID;
        decodedInstruction->detail->sparc.operands[1].mem.disp  = simm;
        std::sprintf(decodedInstruction->op_str, "%s, [%s + %d]", cs::cs_reg_name(m_handle, rd),
                     cs::cs_reg_name(m_handle, rs1), simm);
    }
    else { // reg offset
        const cs::sparc_reg rs2                                 = fixSparcReg(insn & 0x1F);
        decodedInstruction->detail->sparc.operands[1].mem.index = rs2;
        decodedInstruction->detail->sparc.operands[1].mem.disp  = 0;
        std::sprintf(decodedInstruction->op_str, "%s, [%s + %s]", cs::cs_reg_name(m_handle, rd),
                     cs::cs_reg_name(m_handle, rs1), cs::cs_reg_name(m_handle, rs2));
    }

    decodedInstruction->detail->sparc.operands[0].type = cs::SPARC_OP_REG;
    decodedInstruction->detail->sparc.operands[0].reg  = rd;

    Util::writeDWord(&decodedInstruction->bytes, insn, Endian::Little);
    decodedInstruction->bytes[4] = 0;
    std::strcpy(decodedInstruction->mnemonic, "std");
    return true;
}


QString CapstoneSPARCDecoder::getTemplateName(const cs::cs_insn *instruction) const
{
    QString insnID = instruction->mnemonic;

    // chop off branch prediction hints
    if (insnID.endsWith(",pn") || insnID.endsWith(",pt")) {
        insnID.chop(3);
    }

    insnID = insnID.remove(',').toUpper();

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

    return insnID;
}


BOOMERANG_DEFINE_PLUGIN(PluginType::Decoder, CapstoneSPARCDecoder, "Capstone SPARC decoder plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
