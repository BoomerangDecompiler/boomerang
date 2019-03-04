#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SPARCDecoder.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/Proc.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/util/log/Log.h"

#include <cassert>
#include <cstring>


#define DIS_ROI (dis_RegImm(roi))
#define DIS_ADDR (dis_Eaddr(addr))
#define DIS_RD (dis_RegLhs(rd))
#define DIS_RDR (machine->dis_RegRhs(rd))
#define DIS_RS1 (machine->dis_RegRhs(rs1))
#define DIS_FS1S (machine->dis_RegRhs(fs1s + 32))
#define DIS_FS2S (machine->dis_RegRhs(fs2s + 32))
// Note: SPARC V9 has a second set of double precision registers that have an
// odd index. So far we only support V8
#define DIS_FDS (dis_RegLhs(fds + 32))
#define DIS_FS1D (machine->dis_RegRhs((fs1d >> 1) + 64))
#define DIS_FS2D (machine->dis_RegRhs((fs2d >> 1) + 64))
#define DIS_FDD (dis_RegLhs((fdd >> 1) + 64))
#define DIS_FDQ (dis_RegLhs((fdq >> 2) + 80))
#define DIS_FS1Q (machine->dis_RegRhs((fs1q >> 2) + 80))
#define DIS_FS2Q (machine->dis_RegRhs((fs2q >> 2) + 80))


/*
 * addresstoPC returns the raw number as the address.  PC could be an
 * abstract type, in our case, PC is the raw address.
 */
#define addressToPC(pc) pc


void _DEBUG_STMTS(DecodeResult &result, bool debugDecoder)
{
    if (debugDecoder) {
        OStream q_cout(stdout);
        for (Statement *s : *result.rtl) {
            q_cout << "            " << s << "\n";
        }
    }
}

#define DEBUG_STMTS(result) _DEBUG_STMTS(result, m_prog->getProject()->getSettings()->debugDecoder)


std::unique_ptr<RTL> SPARCDecoder::createBranchRTL(const char *insnName, Address pc,
                                                   std::unique_ptr<RTL> stmts)
{
    BranchStatement *br = new BranchStatement();

    stmts->append(br);

    if (insnName[0] == 'F') {
        // fbranch is any of [ FBN FBNE FBLG FBUL FBL    FBUG FBG   FBU
        //                       FBA FBE    FBUE FBGE FBUGE FBLE FBULE FBO ],
        // fbranches are not the same as ibranches, so need a whole different set of tests
        if (insnName[2] == 'U') {
            insnName++; // Just ignore unordered (for now)
        }

        switch (insnName[2]) {
        case 'E': // FBE
            br->setCondType(BranchType::JE, true);
            break;

        case 'L':
            if (insnName[3] == 'G') { // FBLG
                br->setCondType(BranchType::JNE, true);
            }
            else if (insnName[3] == 'E') { // FBLE
                br->setCondType(BranchType::JSLE, true);
            }
            else { // FBL
                br->setCondType(BranchType::JSL, true);
            }
            break;

        case 'G':
            if (insnName[3] == 'E') { // FBGE
                br->setCondType(BranchType::JSGE, true);
            }
            else { // FBG
                br->setCondType(BranchType::JSG, true);
            }

            break;

        case 'N':
            if (insnName[3] == 'E') { // FBNE
                br->setCondType(BranchType::JNE, true);
            }

            // Else it's FBN!
            break;

        default: LOG_WARN("Unknown float branch '%1'", insnName); stmts.reset();
        }

        return stmts;
    }

    // ibranch is any of [ BN BE  BLE BL  BLEU BCS BNEG BVS
    //                       BA BNE BG  BGE BGU  BCC BPOS BVC ],
    // Note: BPN, BPE, etc handled below
    switch (insnName[1]) {
    case 'E':
        br->setCondType(BranchType::JE); // BE
        break;

    case 'L':
        if (insnName[2] == 'E') {
            if (insnName[3] == 'U') {
                br->setCondType(BranchType::JULE); // BLEU
            }
            else {
                br->setCondType(BranchType::JSLE); // BLE
            }
        }
        else {
            br->setCondType(BranchType::JSL); // BL
        }

        break;

    case 'N':
        // BNE, BNEG (won't see BN)
        if (insnName[3] == 'G') {
            br->setCondType(BranchType::JMI); // BNEG
        }
        else {
            br->setCondType(BranchType::JNE); // BNE
        }
        break;

    case 'C':
        // BCC, BCS
        if (insnName[2] == 'C') {
            br->setCondType(BranchType::JUGE); // BCC
        }
        else {
            br->setCondType(BranchType::JUL); // BCS
        }
        break;

    case 'V':
        // BVC, BVS; should never see these now
        if (insnName[2] == 'C') {
            LOG_WARN("Decoded BVC instruction"); // BVC
        }
        else {
            LOG_WARN("Decoded BVS instruction"); // BVS
        }
        break;

    case 'G':
        // BGE, BG, BGU
        if (insnName[2] == 'E') {
            br->setCondType(BranchType::JSGE); // BGE
        }
        else if (insnName[2] == 'U') {
            br->setCondType(BranchType::JUG); // BGU
        }
        else {
            br->setCondType(BranchType::JSG); // BG
        }
        break;

    case 'P':

        if (insnName[2] == 'O') {
            br->setCondType(BranchType::JPOS); // BPOS
            break;
        }

        // Else, it's a BPXX; remove the P (for predicted) and try again
        // (recurse)
        // B P P O S ...
        // 0 1 2 3 4 ...
        char tempName[8];
        tempName[0] = 'B';
        strcpy(tempName + 1, insnName + 2);
        return createBranchRTL(tempName, pc, std::move(stmts));

    default: LOG_WARN("Unknown non-float branch '%1'", insnName);
    }

    return stmts;
}


bool SPARCDecoder::decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &inst)
{
    inst.reset();
    inst.rtl.reset(new RTL(pc));

    HostAddress hostPC = HostAddress(delta) + pc;
    HostAddress nextPC = HostAddress::INVALID;

    // #line 212 "frontend/machine/sparc/decoder.m"
    {
        HostAddress MATCH_p =
            // #line 212 "frontend/machine/sparc/decoder.m"
            hostPC;

        const char *MATCH_name;
        static const char *MATCH_name_cond_0[] = {
            "BPN",   "BPE",  "BPLE", "BPL",  "BPLEU", "BPCS", "BPNEG", "BPVS",
            "BPA,a", "BPNE", "BPG",  "BPGE", "BPGU",  "BPCC", "BPPOS", "BPVC",
        };
        static const char *MATCH_name_cond_1[] = {
            "BPN,a", "BPE,a",  "BPLE,a", "BPL,a",  "BPLEU,a", "BPCS,a", "BPNEG,a", "BPVS,a",
            "BA",    "BPNE,a", "BPG,a",  "BPGE,a", "BPGU,a",  "BPCC,a", "BPPOS,a", "BPVC,a",
        };
        static const char *MATCH_name_cond_2[] = {
            "BN",   "BE",  "BLE", "BL",  "BLEU", "BCS", "BNEG", "BVS",
            "BA,a", "BNE", "BG",  "BGE", "BGU",  "BCC", "BPOS", "BVC",
        };
        static const char *MATCH_name_cond_3[] = {
            "BN,a", "BE,a",  "BLE,a", "BL,a",  "BLEU,a", "BCS,a", "BNEG,a", "BVS,a",
            "FBA",  "BNE,a", "BG,a",  "BGE,a", "BGU,a",  "BCC,a", "BPOS,a", "BVC,a",
        };
        static const char *MATCH_name_cond_5[] = {
            "FBN",   "FBNE", "FBLG", "FBUL", "FBL",   "FBUG", "FBG",   "FBU",
            "FBA,a", "FBE",  "FBUE", "FBGE", "FBUGE", "FBLE", "FBULE", "FBO",
        };
        static const char *MATCH_name_cond_6[] = {
            "FBN,a", "FBNE,a", "FBLG,a", "FBUL,a", "FBL,a",   "FBUG,a", "FBG,a",   "FBU,a",
            "CBA",   "FBE,a",  "FBUE,a", "FBGE,a", "FBUGE,a", "FBLE,a", "FBULE,a", "FBO,a",
        };
        static const char *MATCH_name_cond_7[] = {
            "CBN",   "CB123", "CB12", "CB13", "CB1",   "CB23", "CB2",   "CB3",
            "CBA,a", "CB0",   "CB03", "CB02", "CB023", "CB01", "CB013", "CB012",
        };
        static const char *MATCH_name_cond_8[] = {
            "CBN,a", "CB123,a", "CB12,a", "CB13,a", "CB1,a",   "CB23,a", "CB2,a",   "CB3,a",
            "TA",    "CB0,a",   "CB03,a", "CB02,a", "CB023,a", "CB01,a", "CB013,a", "CB012,a",
        };
        static const char *MATCH_name_op3_46[] = {
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, "RDPSR", "RDWIM", "RDTBR",
        };
        static const char *MATCH_name_opf_51[] = {
            nullptr, "FMOVs", nullptr, nullptr, nullptr,  "FNEGs",  nullptr,  nullptr,  nullptr,
            "FABSs", nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  "FSQRTs", "FSQRTd", "FSQRTq", nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, "FADDs", "FADDd", "FADDq",  nullptr,  "FSUBs",  "FSUBd",  "FSUBq",
            nullptr, "FMULs", "FMULd", "FMULq", nullptr,  "FDIVs",  "FDIVd",  "FDIVq",  nullptr,
            "FCMPs", "FCMPd", "FCMPq", nullptr, "FCMPEs", "FCMPEd", "FCMPEq", nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  nullptr,  nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,  nullptr,  nullptr,  "FiTOs",  nullptr,
            "FdTOs", "FqTOs", "FiTOd", "FsTOd", nullptr,  "FqTOd",  "FiTOq",  "FsTOq",  "FdTOq",
            nullptr, nullptr, "FsTOi", "FdTOi", "FqTOi",
        };
        static const char *MATCH_name_cond_53[] = {
            "TN",    "TE",  "TLE", "TL",  "TLEU", "TCS", "TNEG", "TVS",
            nullptr, "TNE", "TG",  "TGE", "TGU",  "TCC", "TPOS", "TVC",
        };
        static const char *MATCH_name_i_66[] = {
            "LDA",
            "LDF",
        };
        static const char *MATCH_name_i_67[] = {
            "LDUBA",
            "LDFSR",
        };
        static const char *MATCH_name_i_68[] = {
            "LDUHA",
            "LDDF",
        };
        static const char *MATCH_name_i_69[] = {
            "LDDA",
            "STF",
        };
        static const char *MATCH_name_i_70[] = {
            "STA",
            "STFSR",
        };
        static const char *MATCH_name_i_71[] = {
            "STBA",
            "STDFQ",
        };
        static const char *MATCH_name_i_72[] = {
            "STHA",
            "STDF",
        };
        static const char *MATCH_name_i_73[] = {
            "STDA",
            "LDCSR",
        };
        static const char *MATCH_name_i_74[] = {
            "LDSBA",
            "STCSR",
        };
        static const char *MATCH_name_i_75[] = {
            "LDSHA",
            "STDCQ",
        };
        unsigned MATCH_w_32_0;
        {
            MATCH_w_32_0 = getDword(MATCH_p);

            switch ((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */) {
            case 0:

                switch ((MATCH_w_32_0 >> 22 & 0x7) /* op2 at 0 */) {
                case 0: {
                    unsigned n = (MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */;
                    nextPC     = MATCH_p + 4;
                    // #line 629 "frontend/machine/sparc/decoder.m"

                    Q_UNUSED(n);
                    inst.valid = false;
                } break;

                case 1:

                    if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1) {
                        if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) {
                            MATCH_name = MATCH_name_cond_0[(MATCH_w_32_0 >> 25 & 0xf)
                                                           /* cond at 0 */];
                            goto MATCH_label_d0;
                        } /*opt-block*/
                        else {
                            MATCH_name = MATCH_name_cond_1[(MATCH_w_32_0 >> 25 & 0xf)
                                                           /* cond at 0 */];
                            goto MATCH_label_d0;
                        } /*opt-block*/ /*opt-block+*/
                    }
                    else if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) {
                        unsigned cc01   = (MATCH_w_32_0 >> 20 & 0x3) /* cc01 at 0 */;
                        HostAddress tgt = addressToPC(MATCH_p) +
                                          4 * Util::signExtend<sint64>(
                                                  (MATCH_w_32_0 & 0x7ffff) /* disp19 at 0 */, 19);
                        nextPC = MATCH_p + 4;
                        // #line 400 "frontend/machine/sparc/decoder.m"
                        /* Can see bpa xcc,tgt in 32 bit code */

                        Q_UNUSED(cc01); // Does not matter because is unconditional

                        GotoStatement *jump = new GotoStatement;
                        jump->setDest(Address(tgt.value() - delta));

                        inst.type = SD;
                        inst.rtl->append(jump);
                        SHOW_ASM("BPA " << tgt - delta)

                        DEBUG_STMTS(inst);
                    } /*opt-block*/ /*opt-block+*/
                    else {
                        MATCH_name = MATCH_name_cond_0[(MATCH_w_32_0 >> 25 & 0xf)
                                                       /* cond at 0 */];
                        {
                            const char *name = MATCH_name;
                            unsigned cc01    = (MATCH_w_32_0 >> 20 & 0x3) /* cc01 at 0 */;
                            HostAddress tgt  = addressToPC(MATCH_p) +
                                              4 * Util::signExtend<sint64>(
                                                      (MATCH_w_32_0 & 0x7ffff) /* disp19 at 0 */,
                                                      19);
                            nextPC = MATCH_p + 4;
                            // #line 411 "frontend/machine/sparc/decoder.m"

                            if (cc01 != 0) { /* If 64 bit cc used, can't handle */
                                inst.valid = false;
                                inst.rtl.reset(new RTL(Address::INVALID));
                                inst.numBytes = 4;
                                return false;
                            }

                            GotoStatement *jump = nullptr;
                            std::unique_ptr<RTL> rtl;

                            if (strcmp(name, "BPN") == 0) {
                                rtl->append(new GotoStatement);
                            }
                            else if ((strcmp(name, "BPVS") == 0) || (strcmp(name, "BPVC") == 0)) {
                                jump = new GotoStatement;
                                rtl->append(new GotoStatement);
                            }
                            else {
                                rtl = createBranchRTL(name, pc, std::move(inst.rtl));
                                // The BranchStatement will be the last Stmt of the rtl

                                jump = static_cast<GotoStatement *>(rtl->back());
                            }

                            // The class of this instruction depends on whether or not

                            // it is one of the 'unconditional' conditional branches

                            // "BPN" (or the pseudo unconditionals BPVx)

                            inst.type = SCD;

                            if (strcmp(name, "BPVC") == 0) {
                                inst.type = SD;
                            }

                            if ((strcmp(name, "BPN") == 0) || (strcmp(name, "BPVS") == 0)) {
                                inst.type = NCT;
                            }

                            inst.rtl = std::move(rtl);
                            jump->setDest(Address(tgt.value() - delta));
                            SHOW_ASM(name << " " << tgt - delta)
                            DEBUG_STMTS(inst);
                        }
                    } /*opt-block*/ /*opt-block+*/

                    break;

                case 2:

                    if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1) {
                        if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) {
                            MATCH_name = MATCH_name_cond_2[(MATCH_w_32_0 >> 25 & 0xf)];
                        } /*opt-block*/
                        else {
                            MATCH_name = MATCH_name_cond_3[(MATCH_w_32_0 >> 25 & 0xf)];
                        } /*opt-block*/ /*opt-block+*/

                        goto MATCH_label_d2;
                    }
                    else {
                        if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) {
                            MATCH_name = MATCH_name_cond_1[(MATCH_w_32_0 >> 25 & 0xf)];
                        } /*opt-block*/
                        else {
                            MATCH_name = MATCH_name_cond_2[(MATCH_w_32_0 >> 25 & 0xf)];
                        } /*opt-block*/ /*opt-block+*/

                        goto MATCH_label_d1;
                    }

                case 3:
                case 5: goto MATCH_label_d3;

                case 4:

                    if (((MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */ == 0) &&
                        ((MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 0)) {
                        MATCH_name = "NOP";
                        {
                            const char *name = MATCH_name;
                            nextPC           = MATCH_p + 4;
                            // #line 481 "frontend/machine/sparc/decoder.m"

                            inst.type = NOP;
                            inst.rtl  = instantiate(pc, name);
                        }
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d4; /*opt-block+*/
                    }

                    break;

                case 6:

                    if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1) {
                        if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) {
                            MATCH_name = MATCH_name_cond_5[(MATCH_w_32_0 >> 25 & 0xf)];
                        } /*opt-block*/
                        else {
                            MATCH_name = MATCH_name_cond_6[(MATCH_w_32_0 >> 25 & 0xf)];
                        } /*opt-block*/ /*opt-block+*/

                        goto MATCH_label_d2;
                    }
                    else {
                        if ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8) {
                            MATCH_name = MATCH_name_cond_3[(MATCH_w_32_0 >> 25 & 0xf)];
                        } /*opt-block*/
                        else {
                            MATCH_name = MATCH_name_cond_5[(MATCH_w_32_0 >> 25 & 0xf)];
                        } /*opt-block*/ /*opt-block+*/

                        goto MATCH_label_d1;
                    }

                case 7: {
                    uint8_t idx = ((MATCH_w_32_0 >> 25) & 0xf);

                    if ((MATCH_w_32_0 >> 29 & 0x1) /* a at 0 */ == 1) {
                        if (idx == 8) {
                            MATCH_name = MATCH_name_cond_7[idx];
                        }
                        else {
                            MATCH_name = MATCH_name_cond_8[idx];
                        } /*opt-block*/ /*opt-block+*/

                        goto MATCH_label_d2;
                    }
                    else {
                        if (idx == 8) {
                            MATCH_name = MATCH_name_cond_6[idx];
                        } /*opt-block*/
                        else {
                            MATCH_name = MATCH_name_cond_7[idx];
                        } /*opt-block*/ /*opt-block+*/

                        goto MATCH_label_d1;
                    }
                }

                default: assert(0);
                } /* (MATCH_w_32_0 >> 22 & 0x7) -- op2 at 0 --*/

                break;

            case 1: {
                HostAddress addr = addressToPC(MATCH_p) +
                                   4 * Util::signExtend<sint64>(
                                           (MATCH_w_32_0 & 0x3fffffff) /* disp30 at 0 */, 30);
                nextPC = MATCH_p + 4;
                // #line 215 "frontend/machine/sparc/decoder.m"

                /*
                 *
                 * A standard call
                 *
                 */

                CallStatement *newCall = new CallStatement;
                // Set the destination

                Address nativeDest = Address(addr.value() - delta);
                newCall->setDest(nativeDest);
                Function *destProc = m_prog->getOrCreateFunction(nativeDest);

                if (destProc == reinterpret_cast<Function *>(-1)) {
                    destProc = nullptr;
                }

                newCall->setDestProc(destProc);
                inst.rtl->append(newCall);
                inst.type = SD;
                SHOW_ASM("call__ " << (nativeDest))

                DEBUG_STMTS(inst);
            } break;

            case 2:

                switch ((MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */) {
                case 0: MATCH_name = "ADD"; goto MATCH_label_d5;

                case 1: MATCH_name = "AND"; goto MATCH_label_d5;

                case 2: MATCH_name = "OR"; goto MATCH_label_d5;

                case 3: MATCH_name = "XOR"; goto MATCH_label_d5;

                case 4: MATCH_name = "SUB"; goto MATCH_label_d5;

                case 5: MATCH_name = "ANDN"; goto MATCH_label_d5;

                case 6: MATCH_name = "ORN"; goto MATCH_label_d5;

                case 7: MATCH_name = "XNOR"; goto MATCH_label_d5;

                case 8: MATCH_name = "ADDX"; goto MATCH_label_d5;

                case 9:
                case 13:
                case 25:
                case 29:
                case 44:
                case 45:
                case 46:
                case 47:
                case 54:
                case 55:
                case 59:
                case 62:
                case 63: goto MATCH_label_d3;

                case 10: MATCH_name = "UMUL"; goto MATCH_label_d5;

                case 11: MATCH_name = "SMUL"; goto MATCH_label_d5;

                case 12: MATCH_name = "SUBX"; goto MATCH_label_d5;

                case 14: MATCH_name = "UDIV"; goto MATCH_label_d5;

                case 15: MATCH_name = "SDIV"; goto MATCH_label_d5;

                case 16: MATCH_name = "ADDcc"; goto MATCH_label_d5;

                case 17: MATCH_name = "ANDcc"; goto MATCH_label_d5;

                case 18: MATCH_name = "ORcc"; goto MATCH_label_d5;

                case 19: MATCH_name = "XORcc"; goto MATCH_label_d5;

                case 20: MATCH_name = "SUBcc"; goto MATCH_label_d5;

                case 21: MATCH_name = "ANDNcc"; goto MATCH_label_d5;

                case 22: MATCH_name = "ORNcc"; goto MATCH_label_d5;

                case 23: MATCH_name = "XNORcc"; goto MATCH_label_d5;

                case 24: MATCH_name = "ADDXcc"; goto MATCH_label_d5;

                case 26: MATCH_name = "UMULcc"; goto MATCH_label_d5;

                case 27: MATCH_name = "SMULcc"; goto MATCH_label_d5;

                case 28: MATCH_name = "SUBXcc"; goto MATCH_label_d5;

                case 30: MATCH_name = "UDIVcc"; goto MATCH_label_d5;

                case 31: MATCH_name = "SDIVcc"; goto MATCH_label_d5;

                case 32: MATCH_name = "TADDcc"; goto MATCH_label_d5;

                case 33: MATCH_name = "TSUBcc"; goto MATCH_label_d5;

                case 34: MATCH_name = "TADDccTV"; goto MATCH_label_d5;

                case 35: MATCH_name = "TSUBccTV"; goto MATCH_label_d5;

                case 36: MATCH_name = "MULScc"; goto MATCH_label_d5;

                case 37: MATCH_name = "SLL"; goto MATCH_label_d5;

                case 38: MATCH_name = "SRL"; goto MATCH_label_d5;

                case 39: MATCH_name = "SRA"; goto MATCH_label_d5;

                case 40:

                    if ((MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0) {
                        MATCH_name = "RDY";
                        {
                            const char *name = MATCH_name;
                            unsigned rd      = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                            nextPC           = MATCH_p + 4;
                            // #line 533 "frontend/machine/sparc/decoder.m"

                            inst.rtl = instantiate(pc, name, { DIS_RD });
                        }
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                    break;

                case 41:
                    MATCH_name = MATCH_name_op3_46[(MATCH_w_32_0 >> 19 & 0x3f)
                                                   /* op3 at 0 */];
                    {
                        const char *name = MATCH_name;
                        unsigned rd      = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                        nextPC           = MATCH_p + 4;
                        // #line 536 "frontend/machine/sparc/decoder.m"

                        inst.rtl = instantiate(pc, name, { DIS_RD });
                    }

                    break;

                case 42:
                    MATCH_name = MATCH_name_op3_46[(MATCH_w_32_0 >> 19 & 0x3f)
                                                   /* op3 at 0 */];
                    {
                        const char *name = MATCH_name;
                        unsigned rd      = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                        nextPC           = MATCH_p + 4;
                        // #line 539 "frontend/machine/sparc/decoder.m"

                        inst.rtl = instantiate(pc, name, { DIS_RD });
                    }

                    break;

                case 43:
                    MATCH_name = MATCH_name_op3_46[(MATCH_w_32_0 >> 19 & 0x3f)
                                                   /* op3 at 0 */];
                    {
                        const char *name = MATCH_name;
                        unsigned rd      = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                        nextPC           = MATCH_p + 4;
                        // #line 542 "frontend/machine/sparc/decoder.m"

                        inst.rtl = instantiate(pc, name, { DIS_RD });
                    }

                    break;

                case 48:

                    if ((1 <= (MATCH_w_32_0 >> 25 & 0x1f)) /* rd at 0 */ &&
                        ((MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ < 32)) {
                        goto MATCH_label_d3; /*opt-block+*/
                    }
                    else {
                        MATCH_name = "WRY";
                        {
                            const char *name = MATCH_name;
                            HostAddress roi  = addressToPC(MATCH_p);
                            unsigned rs1     = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                            nextPC           = MATCH_p + 4;
                            // #line 545 "frontend/machine/sparc/decoder.m"

                            inst.rtl = instantiate(pc, name, { DIS_RS1, DIS_ROI });
                        }
                    } /*opt-block*/

                    break;

                case 49:
                    MATCH_name = "WRPSR";
                    {
                        const char *name = MATCH_name;
                        HostAddress roi  = addressToPC(MATCH_p);
                        unsigned rs1     = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                        nextPC           = MATCH_p + 4;
                        // #line 548 "frontend/machine/sparc/decoder.m"

                        inst.rtl = instantiate(pc, name, { DIS_RS1, DIS_ROI });
                    }

                    break;

                case 50:
                    MATCH_name = "WRWIM";
                    {
                        const char *name = MATCH_name;
                        HostAddress roi  = addressToPC(MATCH_p);
                        unsigned rs1     = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                        nextPC           = MATCH_p + 4;
                        // #line 551 "frontend/machine/sparc/decoder.m"

                        inst.rtl = instantiate(pc, name, { DIS_RS1, DIS_ROI });
                    }

                    break;

                case 51:
                    MATCH_name = "WRTBR";
                    {
                        const char *name = MATCH_name;
                        HostAddress roi  = addressToPC(MATCH_p);
                        unsigned rs1     = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                        nextPC           = MATCH_p + 4;
                        // #line 554 "frontend/machine/sparc/decoder.m"

                        inst.rtl = instantiate(pc, name, { DIS_RS1, DIS_ROI });
                    }

                    break;

                case 52:

                    if (((80 <= (MATCH_w_32_0 >> 5 & 0x1ff)) /* opf at 0 */ &&
                         ((MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */ < 196)) ||
                        ((MATCH_w_32_0 >> 5 & 0x1ff) > 211)) {
                        goto MATCH_label_d3; /*opt-block+*/
                    }
                    else {
                        switch ((MATCH_w_32_0 >> 5 & 0x1ff) /* opf at 0 */) {
                        case 0:
                        case 2:
                        case 3:
                        case 4:
                        case 6:
                        case 7:
                        case 8:
                        case 10:
                        case 11:
                        case 12:
                        case 13:
                        case 14:
                        case 15:
                        case 16:
                        case 17:
                        case 18:
                        case 19:
                        case 20:
                        case 21:
                        case 22:
                        case 23:
                        case 24:
                        case 25:
                        case 26:
                        case 27:
                        case 28:
                        case 29:
                        case 30:
                        case 31:
                        case 32:
                        case 33:
                        case 34:
                        case 35:
                        case 36:
                        case 37:
                        case 38:
                        case 39:
                        case 40:
                        case 44:
                        case 45:
                        case 46:
                        case 47:
                        case 48:
                        case 49:
                        case 50:
                        case 51:
                        case 52:
                        case 53:
                        case 54:
                        case 55:
                        case 56:
                        case 57:
                        case 58:
                        case 59:
                        case 60:
                        case 61:
                        case 62:
                        case 63:
                        case 64:
                        case 68:
                        case 72:
                        case 76:
                        case 197:
                        case 202:
                        case 207:
                        case 208: goto MATCH_label_d3;

                        case 1:
                        case 5:
                        case 9:
                        case 41:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fds     = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                                unsigned fs2s    = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 560 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2S, DIS_FDS });
                            }

                            break;

                        case 42:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fdd     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                                unsigned fs2d    = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 611 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2D, DIS_FDD });
                            }

                            break;

                        case 43:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fdq     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                                unsigned fs2q    = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 614 "frontend/machine/sparc/decoder.m"
                                inst.rtl = instantiate(pc, name, { DIS_FS2Q, DIS_FDQ });
                                // In V9, the privileged RETT becomes user-mode RETURN
                                // It has the semantics of "ret restore" without the add part of the
                                // restore
                            }

                            break;

                        case 65:
                        case 69:
                        case 73:
                        case 77:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fds     = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                                unsigned fs1s    = (MATCH_w_32_0 >> 14 & 0x1f) /* fs1s at 0 */;
                                unsigned fs2s    = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 563 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS1S, DIS_FS2S, DIS_FDS });
                            }

                            break;

                        case 66:
                        case 70:
                        case 74:
                        case 78:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fdd     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                                unsigned fs1d    = (MATCH_w_32_0 >> 14 & 0x1f) /* fs1d at 0 */;
                                unsigned fs2d    = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 566 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS1D, DIS_FS2D, DIS_FDD });
                            }

                            break;

                        case 67:
                        case 71:
                        case 75:
                        case 79:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fdq     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                                unsigned fs1q    = (MATCH_w_32_0 >> 14 & 0x1f) /* fs1q at 0 */;
                                unsigned fs2q    = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 569 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS1Q, DIS_FS2Q, DIS_FDQ });
                            }

                            break;

                        case 196:
                        case 209:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fds     = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                                unsigned fs2s    = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 581 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2S, DIS_FDS });
                                // Note: itod and dtoi have different sized registers
                            }

                            break;

                        case 198:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fds     = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                                unsigned fs2d    = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 597 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2D, DIS_FDS });
                            }

                            break;

                        case 199:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fds     = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                                unsigned fs2q    = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 602 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2Q, DIS_FDS });
                            }

                            break;

                        case 200:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fdd     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                                unsigned fs2s    = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 584 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2S, DIS_FDD });
                            }

                            break;

                        case 201:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fdd     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                                unsigned fs2s    = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 594 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2S, DIS_FDD });
                            }

                            break;

                        case 203:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fdd     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
                                unsigned fs2q    = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 607 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2Q, DIS_FDD });
                            }

                            break;

                        case 204:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fdq     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                                unsigned fs2s    = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 589 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2S, DIS_FDQ });
                            }

                            break;

                        case 205:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fdq     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                                unsigned fs2s    = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 599 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2S, DIS_FDQ });
                            }

                            break;

                        case 206:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fdq     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdq at 0 */;
                                unsigned fs2d    = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 604 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2D, DIS_FDQ });
                            }

                            break;

                        case 210:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fds     = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                                unsigned fs2d    = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 587 "frontend/machine/sparc/decoder.m"

                                inst.rtl = instantiate(pc, name, { DIS_FS2D, DIS_FDS });
                            }

                            break;

                        case 211:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)
                                                           /* opf at 0 */];
                            {
                                const char *name = MATCH_name;
                                unsigned fds     = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
                                unsigned fs2q    = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                                nextPC           = MATCH_p + 4;
                                // #line 592 "frontend/machine/sparc/decoder.m"
                                inst.rtl = instantiate(pc, name, { DIS_FS2Q, DIS_FDS });
                            }

                            break;

                        default: assert(0);
                        } /* (MATCH_w_32_0 >> 5 & 0x1ff) -- opf at 0 --*/
                    }

                    break;

                case 53: {
                    uint32_t matched_val = ((MATCH_w_32_0 >> 5) & 0x1ff); // 0-511

                    if ((matched_val < 81) || (matched_val >= 88)) {
                        goto MATCH_label_d3; /*opt-block+*/
                    }
                    else {
                        switch (matched_val) {
                        case 84: goto MATCH_label_d3;

                        case 81:
                        case 85:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)];
                            {
                                const char *name = MATCH_name;
                                unsigned fs1s    = (MATCH_w_32_0 >> 14 & 0x1f) /* fs1s at 0 */;
                                unsigned fs2s    = (MATCH_w_32_0 & 0x1f) /* fs2s at 0 */;
                                nextPC           = MATCH_p + 4;
                                inst.rtl         = instantiate(pc, name, { DIS_FS1S, DIS_FS2S });
                            }
                            break;

                        case 82:
                        case 86:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)];
                            {
                                const char *name = MATCH_name;
                                unsigned fs1d    = (MATCH_w_32_0 >> 14 & 0x1f) /* fs1d at 0 */;
                                unsigned fs2d    = (MATCH_w_32_0 & 0x1f) /* fs2d at 0 */;
                                nextPC           = MATCH_p + 4;
                                inst.rtl         = instantiate(pc, name, { DIS_FS1D, DIS_FS2D });
                            }
                            break;

                        case 83:
                        case 87:
                            MATCH_name = MATCH_name_opf_51[(MATCH_w_32_0 >> 5 & 0x1ff)];
                            {
                                const char *name = MATCH_name;
                                unsigned fs1q    = (MATCH_w_32_0 >> 14 & 0x1f) /* fs1q at 0 */;
                                unsigned fs2q    = (MATCH_w_32_0 & 0x1f) /* fs2q at 0 */;
                                nextPC           = MATCH_p + 4;
                                inst.rtl         = instantiate(pc, name, { DIS_FS1Q, DIS_FS2Q });
                            }
                            break;

                        default: assert(0);
                        } /* (MATCH_w_32_0 >> 5 & 0x1ff) -- opf at 0 --*/
                    }
                } break;

                case 56:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                        switch ((MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */) {
                        case 0:

                            switch ((MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                            case 4:
                            case 5:
                            case 6:
                            case 7:
                            case 8:
                            case 9:
                            case 10:
                            case 11:
                            case 12:
                            case 13:
                            case 14:
                            case 16:
                            case 17:
                            case 18:
                            case 19:
                            case 20:
                            case 21:
                            case 22:
                            case 23:
                            case 24:
                            case 25:
                            case 26:
                            case 27:
                            case 28:
                            case 29:
                            case 30: goto MATCH_label_d6;

                            case 15:

                                if ((MATCH_w_32_0 & 0x1fff)
                                    /* simm13 at 0 */
                                    == 8) {
                                    nextPC = MATCH_p + 4;
                                    // #line 262 "frontend/machine/sparc/decoder.m"

                                    /*
                                     *
                                     * Just a ret (leaf; uses %o7 instead of %i7)
                                     *
                                     */

                                    inst.rtl->append(new ReturnStatement);
                                    inst.type = DD;
                                    SHOW_ASM("retl_")

                                    DEBUG_STMTS(inst);
                                } /*opt-block*/ /*opt-block+*/
                                else {
                                    goto MATCH_label_d6; /*opt-block+*/
                                }

                                break;

                            case 31:

                                if ((MATCH_w_32_0 & 0x1fff)
                                    /* simm13 at 0 */
                                    == 8) {
                                    nextPC = MATCH_p + 4;
                                    // #line 252 "frontend/machine/sparc/decoder.m"

                                    /*
                                     *
                                     * Just a ret (non leaf)
                                     *
                                     */

                                    inst.rtl->append(new ReturnStatement);
                                    inst.type = DD;
                                    SHOW_ASM("ret_")

                                    DEBUG_STMTS(inst);
                                } /*opt-block*/ /*opt-block+*/
                                else {
                                    goto MATCH_label_d6; /*opt-block+*/
                                }

                                break;

                            default: assert(0);
                            } /* (MATCH_w_32_0 >> 14 & 0x1f) -- rs1 at 0 --*/

                            break;

                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 9:
                        case 10:
                        case 11:
                        case 12:
                        case 13:
                        case 14:
                        case 16:
                        case 17:
                        case 18:
                        case 19:
                        case 20:
                        case 21:
                        case 22:
                        case 23:
                        case 24:
                        case 25:
                        case 26:
                        case 27:
                        case 28:
                        case 29:
                        case 30:
                        case 31: goto MATCH_label_d6;

                        case 15: goto MATCH_label_d7;

                        default: assert(0);
                        } /* (MATCH_w_32_0 >> 25 & 0x1f) -- rd at 0 --*/
                    }
                    else if ((MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */ == 15) {
                        goto MATCH_label_d7; /*opt-block+*/
                    }
                    else {
                        goto MATCH_label_d6; /*opt-block+*/ /*opt-block+*/
                    }

                    break;

                case 57:
                    MATCH_name = "RETURN";
                    {
                        const char *name = MATCH_name;
                        HostAddress addr = addressToPC(MATCH_p);
                        nextPC           = MATCH_p + 4;
                        // #line 620 "frontend/machine/sparc/decoder.m"
                        inst.rtl = instantiate(pc, name, { DIS_ADDR });
                        inst.rtl->append(new ReturnStatement);
                        inst.type = DD;
                    }

                    break;

                case 58:

                    if (((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2) &&
                        ((MATCH_w_32_0 >> 25 & 0xf) /* cond at 0 */ == 8)) {
                        MATCH_name = MATCH_name_cond_8[(MATCH_w_32_0 >> 25 & 0xf)];
                    } /*opt-block*/
                    else {
                        MATCH_name = MATCH_name_cond_53[(MATCH_w_32_0 >> 25 & 0xf)];
                    } /*opt-block*/

                    goto MATCH_label_d8;

                case 60: {
                    unsigned rd     = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                    HostAddress roi = addressToPC(MATCH_p);
                    unsigned rs1    = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                    nextPC          = MATCH_p + 4;
                    // #line 471 "frontend/machine/sparc/decoder.m"

                    // Decided to treat SAVE as an ordinary instruction
                    // That is, use the large list of effects from the SSL file, and
                    // hope that optimisation will vastly help the common cases
                    inst.rtl = instantiate(pc, "SAVE", { DIS_RS1, DIS_ROI, DIS_RD });
                } break;

                case 61: {
                    unsigned rd     = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
                    HostAddress roi = addressToPC(MATCH_p);
                    unsigned rs1    = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                    nextPC          = MATCH_p + 4;
                    // #line 477 "frontend/machine/sparc/decoder.m"

                    // Decided to treat RESTORE as an ordinary instruction

                    inst.rtl = instantiate(pc, "RESTORE", { DIS_RS1, DIS_ROI, DIS_RD });
                } break;

                default: assert(0);
                } /* (MATCH_w_32_0 >> 19 & 0x3f) -- op3 at 0 --*/

                break;

            case 3:

                switch ((MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */) {
                case 0: MATCH_name = "LD"; goto MATCH_label_d9;

                case 1: MATCH_name = "LDUB"; goto MATCH_label_d9;

                case 2: MATCH_name = "LDUH"; goto MATCH_label_d9;

                case 3: MATCH_name = "LDD"; goto MATCH_label_d9;

                case 4: MATCH_name = "ST"; goto MATCH_label_d10;

                case 5: MATCH_name = "STB"; goto MATCH_label_d10;

                case 6: MATCH_name = "STH"; goto MATCH_label_d10;

                case 7: MATCH_name = "STD"; goto MATCH_label_d10;

                case 8:
                case 11:
                case 12:
                case 14:
                case 24:
                case 27:
                case 28:
                case 30:
                case 34:
                case 40:
                case 41:
                case 42:
                case 43:
                case 44:
                case 45:
                case 46:
                case 47:
                case 48:
                case 50:
                case 51:
                case 52:
                case 55:
                case 56:
                case 57:
                case 58:
                case 59:
                case 60:
                case 61:
                case 62:
                case 63: goto MATCH_label_d3;

                case 9: MATCH_name = "LDSB"; goto MATCH_label_d9;

                case 10: MATCH_name = "LDSH"; goto MATCH_label_d9;

                case 13: MATCH_name = "LDSTUB"; goto MATCH_label_d9;

                case 15: MATCH_name = "SWAP."; goto MATCH_label_d9;

                case 16:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = MATCH_name_i_66[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                        goto MATCH_label_d11;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 17:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = MATCH_name_i_67[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                        goto MATCH_label_d11;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 18:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = MATCH_name_i_68[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                        goto MATCH_label_d11;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 19:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = MATCH_name_i_69[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                        goto MATCH_label_d11;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 20:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = MATCH_name_i_70[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                        goto MATCH_label_d12;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 21:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = MATCH_name_i_71[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                        goto MATCH_label_d12;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 22:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = MATCH_name_i_72[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                        goto MATCH_label_d12;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 23:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = MATCH_name_i_73[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                        goto MATCH_label_d12;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 25:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = MATCH_name_i_74[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                        goto MATCH_label_d11;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 26:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = MATCH_name_i_75[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                        goto MATCH_label_d11;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 29:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = "LDSTUBA";
                        goto MATCH_label_d11;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 31:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 0) {
                        MATCH_name = "SWAPA";
                        goto MATCH_label_d11;
                    } /*opt-block*/
                    else {
                        goto MATCH_label_d3; /*opt-block+*/
                    }

                case 32:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                        MATCH_name = MATCH_name_i_66[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                    } /*opt-block*/
                    else {
                        MATCH_name = "LDF";
                    } /*opt-block*/

                    goto MATCH_label_d13;

                case 33:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                        MATCH_name = MATCH_name_i_67[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                    } /*opt-block*/
                    else {
                        MATCH_name = "LDFSR";
                    } /*opt-block*/

                    goto MATCH_label_d14;

                case 35:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                        MATCH_name = MATCH_name_i_68[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                    } /*opt-block*/
                    else {
                        MATCH_name = "LDDF";
                    } /*opt-block*/

                    goto MATCH_label_d15;

                case 36:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                        MATCH_name = MATCH_name_i_69[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                    } /*opt-block*/
                    else {
                        MATCH_name = "STF";
                    } /*opt-block*/

                    goto MATCH_label_d16;

                case 37:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                        MATCH_name = MATCH_name_i_70[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                    } /*opt-block*/
                    else {
                        MATCH_name = "STFSR";
                    } /*opt-block*/

                    goto MATCH_label_d17;

                case 38:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                        MATCH_name = MATCH_name_i_71[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                    } /*opt-block*/
                    else {
                        MATCH_name = "STDFQ";
                    } /*opt-block*/

                    goto MATCH_label_d18;

                case 39:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                        MATCH_name = MATCH_name_i_72[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                    } /*opt-block*/
                    else {
                        MATCH_name = "STDF";
                    } /*opt-block*/

                    goto MATCH_label_d19;

                case 49:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                        MATCH_name = MATCH_name_i_73[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                    } /*opt-block*/
                    else {
                        MATCH_name = "LDCSR";
                    } /*opt-block*/

                    goto MATCH_label_d20;

                case 53:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                        MATCH_name = MATCH_name_i_74[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                    } /*opt-block*/
                    else {
                        MATCH_name = "STCSR";
                    } /*opt-block*/

                    goto MATCH_label_d21;

                case 54:

                    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                        MATCH_name = MATCH_name_i_75[(MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */];
                    } /*opt-block*/
                    else {
                        MATCH_name = "STDCQ";
                    } /*opt-block*/

                    goto MATCH_label_d22;

                default: assert(0);
                } /* (MATCH_w_32_0 >> 19 & 0x3f) -- op3 at 0 --*/

                break;

            default: assert(0);
            } /* (MATCH_w_32_0 >> 30 & 0x3) -- op at 0 --*/
        }
        goto MATCH_finished_d;
    MATCH_label_d0:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            unsigned cc01    = (MATCH_w_32_0 >> 20 & 0x3) /* cc01 at 0 */;
            HostAddress tgt  = addressToPC(MATCH_p) +
                              4 * Util::signExtend<sint64>(
                                      (MATCH_w_32_0 & 0x7ffff) /* disp19 at 0 */, 19);
            nextPC = MATCH_p + 4;
            // #line 316 "frontend/machine/sparc/decoder.m"

            /*
             *
             * Anulled , predicted branch (treat as for non predicted)
             *
             */

            // Instantiate a GotoStatement for the unconditional branches, HLJconds for the rest.

            // NOTE: NJMC toolkit cannot handle embedded else statements!

            if (cc01 != 0) { /* If 64 bit cc used, can't handle */
                inst.valid = false;
                inst.rtl.reset(new RTL(Address::INVALID));
                inst.numBytes = 4;
                return inst.valid;
            }

            GotoStatement *jump = nullptr;

            if ((strcmp(name, "BPA,a") == 0) || (strcmp(name, "BPN,a") == 0)) {
                jump = new GotoStatement;
                inst.rtl->append(jump);
            }
            else if ((strcmp(name, "BPVS,a") == 0) || (strcmp(name, "BPVC,a") == 0)) {
                jump = new GotoStatement;
                inst.rtl->append(jump);
            }
            else {
                inst.rtl = createBranchRTL(name, pc, std::move(inst.rtl));
                jump     = static_cast<GotoStatement *>(inst.rtl->back());
            }

            // The class of this instruction depends on whether or not it is one of the
            // 'unconditional' conditional branches

            // "BPA,A" or "BPN,A"

            inst.type = SCDAN;

            if ((strcmp(name, "BPA,a") == 0) || (strcmp(name, "BPVC,a") == 0)) {
                inst.type = SU;
            }
            else {
                inst.type = SKIP;
            }

            jump->setDest(Address((tgt - delta).value()));
            SHOW_ASM(name << " " << tgt - delta)

            DEBUG_STMTS(inst);
        }
        goto MATCH_finished_d;
    MATCH_label_d1:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress tgt  = addressToPC(MATCH_p) +
                              4 * Util::signExtend<sint64>(
                                      (MATCH_w_32_0 & 0x3fffff) /* disp22 at 0 */, 22);
            nextPC = MATCH_p + 4;
            // #line 358 "frontend/machine/sparc/decoder.m"

            /*
             *
             * Non anulled branch
             *
             */

            // First, check for CBxxx branches (branches that depend on co-processor instructions).
            // These are invalid,

            // as far as we are concerned

            if (name[0] == 'C') {
                inst.valid = false;
                inst.rtl.reset(new RTL(Address::INVALID));
                inst.numBytes = 4;
                return false;
            }

            // Instantiate a GotoStatement for the unconditional branches, BranchStatement for the
            // rest

            // NOTE: NJMC toolkit cannot handle embedded plain else statements! (But OK with curly
            // bracket before the else)

            GotoStatement *jump = nullptr;

            if ((strcmp(name, "BA") == 0) || (strcmp(name, "BN") == 0)) {
                jump = new GotoStatement;
                inst.rtl->append(jump);
            }
            else if ((strcmp(name, "BVS") == 0) || (strcmp(name, "BVC") == 0)) {
                jump = new GotoStatement;
                inst.rtl->append(jump);
            }
            else {
                inst.rtl = createBranchRTL(name, pc, std::move(inst.rtl));
                jump     = static_cast<BranchStatement *>(inst.rtl->back());
            }

            // The class of this instruction depends on whether or not it is one of the
            // 'unconditional' conditional branches

            // "BA" or "BN" (or the pseudo unconditionals BVx)

            inst.type = SCD;

            if ((strcmp(name, "BA") == 0) || (strcmp(name, "BVC") == 0)) {
                inst.type = SD;
            }

            if ((strcmp(name, "BN") == 0) || (strcmp(name, "BVS") == 0)) {
                inst.type = NCT;
            }

            jump->setDest(Address((tgt - delta).value()));
            SHOW_ASM(name << " " << tgt - delta)

            DEBUG_STMTS(inst);
        }
        goto MATCH_finished_d;
    MATCH_label_d2:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress tgt  = addressToPC(MATCH_p) +
                              4 * Util::signExtend<sint64>(
                                      (MATCH_w_32_0 & 0x3fffff) /* disp22 at 0 */, 22);
            nextPC = MATCH_p + 4;
            // #line 272 "frontend/machine/sparc/decoder.m"

            /*
             *
             * Anulled branch
             *
             */

            // First, check for CBxxx branches (branches that depend on co-processor instructions).
            // These are invalid,

            // as far as we are concerned

            if (name[0] == 'C') {
                inst.valid = false;
                inst.rtl.reset(new RTL(Address::INVALID));
                inst.numBytes = 4;
                return false;
            }

            // The class of this instruction depends on whether or not it is one of the
            // 'unconditional' conditional branches

            // "BA,A" or "BN,A"
            if (strcmp(name, "BA,a") == 0) {
                inst.type = SU;
            }
            else if (strcmp(name, "BN,a") == 0) {
                inst.type = SKIP;
            }
            else {
                // ordinary branch instruction
                inst.type = SCDAN;
            }

            // Instantiate a GotoStatement for the unconditional branches,
            // BranchStatements for the rest.
            GotoStatement *jump = nullptr;
            if (inst.type == SU || inst.type == SKIP) {
                jump = new GotoStatement;
                inst.rtl->append(jump);
            }
            else {
                inst.rtl = createBranchRTL(name, pc, std::move(inst.rtl));
                jump     = static_cast<GotoStatement *>(inst.rtl->back());
            }

            jump->setDest(Address((tgt - delta).value()));
            SHOW_ASM(name << " " << tgt - delta)

            DEBUG_STMTS(inst);
        }
        goto MATCH_finished_d;
    MATCH_label_d3:
        (void)0; /*placeholder for label*/
        {
            unsigned n = MATCH_w_32_0 /* inst at 0 */;
            nextPC     = MATCH_p + 4;
            // #line 634 "frontend/machine/sparc/decoder.m"

            // What does this mean?

            Q_UNUSED(n);
            inst.valid = false;
        }
        goto MATCH_finished_d;
    MATCH_label_d4:
        (void)0; /*placeholder for label*/
        {
            unsigned imm22 = (MATCH_w_32_0 & 0x3fffff) /* imm22 at 0 */ << 10;
            unsigned rd    = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
            nextPC         = MATCH_p + 4;
            // #line 485 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, "sethi", { dis_Num(imm22), DIS_RD });
        }
        goto MATCH_finished_d;
    MATCH_label_d5:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            unsigned rd      = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
            HostAddress roi  = addressToPC(MATCH_p);
            unsigned rs1     = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
            nextPC           = MATCH_p + 4;
            // #line 557 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_RS1, DIS_ROI, DIS_RD });
        }
        goto MATCH_finished_d;
    MATCH_label_d6:
        (void)0; /*placeholder for label*/
        {
            HostAddress addr = addressToPC(MATCH_p);
            unsigned rd      = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
            nextPC           = MATCH_p + 4;
            // #line 448 "frontend/machine/sparc/decoder.m"

            /*
             *
             * JMPL, with rd != %o7, i.e. register jump
             *
             * Note: if rd==%o7, then would be handled with the call_ arm
             *
             */

            CaseStatement *jump = new CaseStatement;
            // Record the fact that it is a computed jump

            jump->setIsComputed();
            inst.rtl->append(jump);
            inst.type = DD;
            jump->setDest(dis_Eaddr(addr));
            Q_UNUSED(rd);
            SHOW_ASM("JMPL ")

            DEBUG_STMTS(inst);

            //    //    //    //    //    //    //    //

            //                            //

            //     Ordinary instructions    //

            //                            //

            //    //    //    //    //    //    //    //
        }
        goto MATCH_finished_d;
    MATCH_label_d7:
        (void)0; /*placeholder for label*/
        {
            HostAddress addr = addressToPC(MATCH_p);
            nextPC           = MATCH_p + 4;
            // #line 233 "frontend/machine/sparc/decoder.m"

            /*
             *
             * A JMPL with rd == %o7, i.e. a register call
             *
             */

            CallStatement *newCall = new CallStatement;
            // Record the fact that this is a computed call

            newCall->setIsComputed();
            // Set the destination expression

            newCall->setDest(dis_Eaddr(addr));
            inst.rtl->append(newCall);
            inst.type = DD;
            SHOW_ASM("call_ " << dis_Eaddr(addr))

            DEBUG_STMTS(inst);
        }
        goto MATCH_finished_d;
    MATCH_label_d8:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            nextPC           = MATCH_p + 4;
            // #line 626 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_label_d9:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            unsigned rd      = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
            nextPC           = MATCH_p + 4;
            // #line 488 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_ADDR, DIS_RD });
        }
        goto MATCH_finished_d;
    MATCH_label_d10:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            unsigned rd      = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
            nextPC           = MATCH_p + 4;
            // #line 501 "frontend/machine/sparc/decoder.m"

            // Note: RD is on the "right hand side" only for stores

            inst.rtl = instantiate(pc, name, { DIS_RDR, DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_label_d11:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            unsigned asi     = (MATCH_w_32_0 >> 5 & 0xff) /* asi at 0 */;
            unsigned rd      = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
            nextPC           = MATCH_p + 4;
            // #line 497 "frontend/machine/sparc/decoder.m"

            Q_UNUSED(asi); // Note: this could be serious!

            inst.rtl = instantiate(pc, name, { DIS_RD, DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_label_d12:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            unsigned asi     = (MATCH_w_32_0 >> 5 & 0xff) /* asi at 0 */;
            unsigned rd      = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
            nextPC           = MATCH_p + 4;
            // #line 511 "frontend/machine/sparc/decoder.m"

            Q_UNUSED(asi); // Note: this could be serious!

            inst.rtl = instantiate(pc, name, { DIS_RDR, DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_label_d13:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            unsigned fds     = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
            nextPC           = MATCH_p + 4;
            // #line 491 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_ADDR, DIS_FDS });
        }
        goto MATCH_finished_d;
    MATCH_label_d14:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            nextPC           = MATCH_p + 4;
            // #line 515 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_label_d15:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            unsigned fdd     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
            nextPC           = MATCH_p + 4;
            // #line 494 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_ADDR, DIS_FDD });
        }
        goto MATCH_finished_d;
    MATCH_label_d16:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            unsigned fds     = (MATCH_w_32_0 >> 25 & 0x1f) /* fds at 0 */;
            nextPC           = MATCH_p + 4;
            // #line 505 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_FDS, DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_label_d17:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            nextPC           = MATCH_p + 4;
            // #line 521 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_label_d18:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            nextPC           = MATCH_p + 4;
            // #line 527 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_label_d19:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            unsigned fdd     = (MATCH_w_32_0 >> 25 & 0x1f) /* fdd at 0 */;
            nextPC           = MATCH_p + 4;
            // #line 508 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_FDD, DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_label_d20:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            nextPC           = MATCH_p + 4;
            // #line 518 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_label_d21:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            nextPC           = MATCH_p + 4;
            // #line 524 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_label_d22:
        (void)0; /*placeholder for label*/
        {
            const char *name = MATCH_name;
            HostAddress addr = addressToPC(MATCH_p);
            nextPC           = MATCH_p + 4;
            // #line 530 "frontend/machine/sparc/decoder.m"

            inst.rtl = instantiate(pc, name, { DIS_ADDR });
        }
        goto MATCH_finished_d;
    MATCH_finished_d:
        (void)0; /*placeholder for label*/
    }

    // #line 645 "frontend/machine/sparc/decoder.m"

    inst.numBytes = (nextPC - hostPC).value();
    assert(inst.numBytes > 0);

    return inst.valid;
}


SharedExp SPARCDecoder::dis_RegLhs(unsigned r)
{
    return Location::regOf(r);
}


SharedExp SPARCMachine::dis_RegRhs(uint8_t reg_no)
{
    if (reg_no == 0) {
        return Const::get(0);
    }

    return Location::regOf(reg_no);
}


SharedExp SPARCDecoder::dis_RegImm(HostAddress pc)
{
    HostAddress MATCH_p   = pc;
    unsigned MATCH_w_32_0 = getDword(MATCH_p);

    if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
        int /* [~4096..4095] */ i = Util::signExtend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
        return Const::get(i);
    } /*opt-block*/ /*opt-block+*/
    else {
        unsigned rs2 = (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */;
        return machine->dis_RegRhs(rs2);
    } /*opt-block*/ /*opt-block+*/
}


SharedExp SPARCDecoder::dis_Eaddr(HostAddress pc, int size)
{
    Q_UNUSED(size);
    SharedExp expr;
    // #line 715 "frontend/machine/sparc/decoder.m"
    {
        HostAddress MATCH_p =

            // #line 715 "frontend/machine/sparc/decoder.m"
            pc;

        unsigned MATCH_w_32_0;
        {
            MATCH_w_32_0 = getDword(MATCH_p);

            if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                if ((MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0) {
                    int /* [~4096..4095] */ i = Util::signExtend(
                        (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
                    // #line 722 "frontend/machine/sparc/decoder.m"

                    expr = Const::get(static_cast<int>(i));
                } /*opt-block*/ /*opt-block+*/
                else {
                    int /* [~4096..4095] */ i = Util::signExtend(
                        (MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
                    unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                    // #line 725 "frontend/machine/sparc/decoder.m"

                    expr = Binary::get(opPlus, Location::regOf(rs1), Const::get(i));
                } /*opt-block*/ /*opt-block+*/ /*opt-block+*/
            }
            else if ((MATCH_w_32_0 & 0x1f) /* rs2 at 0 */ == 0) {
                unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                // #line 716 "frontend/machine/sparc/decoder.m"

                expr = Location::regOf(rs1);
            } /*opt-block*/ /*opt-block+*/
            else {
                unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                unsigned rs2 = (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */;
                // #line 719 "frontend/machine/sparc/decoder.m"

                expr = Binary::get(opPlus, Location::regOf(rs1), Location::regOf(rs2));
            } /*opt-block*/ /*opt-block+*/ /*opt-block+*/
        }
        goto MATCH_finished_b;
    MATCH_finished_b:
        (void)0; /*placeholder for label*/
    }

    // #line 730 "frontend/machine/sparc/decoder.m"

    return expr;
}


bool SPARCDecoder::isFuncPrologue(HostAddress hostPC)
{
    Q_UNUSED(hostPC);
    return false;
}


bool SPARCDecoder::isRestore(HostAddress hostPC)
{
    HostAddress MATCH_p   = hostPC;
    unsigned MATCH_w_32_0 = getDword(MATCH_p);

    if (((MATCH_w_32_0 >> 30 & 0x3) /* op at 0 */ == 2) &&
        ((MATCH_w_32_0 >> 19 & 0x3f) /* op3 at 0 */ == 61) &&
        ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ < 2)) {
        unsigned a    = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
        HostAddress b = addressToPC(MATCH_p);
        unsigned c    = (MATCH_w_32_0 >> 25 & 0x1f) /* rd at 0 */;
        Q_UNUSED(a); // Suppress warning messages
        (void)b;
        Q_UNUSED(c);
        return true;
    } /*opt-block*/ /*opt-block+*/

    return false;
}


DWord SPARCDecoder::getDword(HostAddress lc)
{
    return Util::readDWord(lc, Endian::Big);
}


SPARCDecoder::SPARCDecoder(Project *project)
    : NJMCDecoder(project, "ssl/sparc.ssl")
    , machine(new SPARCMachine)
{
}


BOOMERANG_DEFINE_PLUGIN(PluginType::Decoder, SPARCDecoder, "SPARC decoder plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
