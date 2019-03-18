#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ST20Decoder.h"

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
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/util/log/Log.h"

#include <cassert>


bool ST20Decoder::decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result)
{
    result.reset();

    HostAddress hostPC = HostAddress(delta) + pc;

    std::unique_ptr<RTL> stmts; // The actual list of instantiated Statements
    int total = 0;              // Total value from all prefixes

    while (true) {
        HostAddress MATCH_p = hostPC + result.numBytes++;
        const char *MATCH_name;
        static const char *MATCH_name_fc_0[] = {
            nullptr, "ldlp",  nullptr, "ldnl", "ldc", "ldnlp", nullptr, "ldl",
            "adc",   nullptr, nullptr, "ajw",  "eqc", "stl",   "stnl",
        };

        unsigned /* [0..255] */ MATCH_w_8_0;
        {
            MATCH_w_8_0 = Util::readByte((const void *)MATCH_p.value());

            switch ((MATCH_w_8_0 >> 4 & 0xf) /* fc at 0 */) {
            case 0: {
                unsigned oper = (MATCH_w_8_0 & 0xf) /* bot at 0 */;
                processUnconditionalJump("j", result.numBytes,
                                            hostPC + result.numBytes + total + oper, delta, pc,
                                            result);
            } break;

            case 1:
            case 3:
            case 4:
            case 5:
            case 7:
            case 8:
            case 11:
            case 12:
            case 13:
            case 14:
                MATCH_name = MATCH_name_fc_0[(MATCH_w_8_0 >> 4 & 0xf) /* fc at 0 */];
                {
                    const char *name = MATCH_name;
                    unsigned oper    = (MATCH_w_8_0 & 0xf) /* bot at 0 */;
                    result.rtl = instantiate(pc, name, { Const::get(total + oper) });
                }

                break;

            case 2: {
                unsigned oper = (MATCH_w_8_0 & 0xf) /* bot at 0 */;
                total = (total + oper) << 4;

                continue;
            } break;

            case 6: {
                unsigned oper = (MATCH_w_8_0 & 0xf) /* bot at 0 */;
                total = (total + ~oper) << 4;

                continue;
            } break;

            case 9: {
                unsigned oper = (MATCH_w_8_0 & 0xf) /* bot at 0 */;
                total += oper;

                result.rtl = instantiate(pc, "call", { Const::get(total) });

                CallStatement *newCall = new CallStatement;
                newCall->setIsComputed(false);
                newCall->setDest(pc + result.numBytes + total);
                result.rtl->append(newCall);
            } break;

            case 10: {
                unsigned oper = (MATCH_w_8_0 & 0xf) /* bot at 0 */;

                BranchStatement *br = new BranchStatement();
                br->setDest(pc + result.numBytes + total + oper);
                br->setCondExpr(Binary::get(opEquals, dis_Reg(0), Const::get(0)));

                result.rtl = std::move(stmts);
                result.rtl->append(br);
            } break;

            case 15: {
                unsigned oper = (MATCH_w_8_0 & 0xf) /* bot at 0 */;
                total |= oper;

                const char *name = nullptr;

                bool isRet = false;

                if (total >= 0) {
                    switch (total) {
                    case 0x00: name = "rev"; break;
                    case 0x01: name = "lb"; break;
                    case 0x02: name = "bsub"; break;
                    case 0x03: name = "endp"; break;
                    case 0x04: name = "diff"; break;
                    case 0x05: name = "add"; break;
                    case 0x06: name = "gcall"; break;
                    case 0x07: name = "in"; break;
                    case 0x08: name = "prod"; break;
                    case 0x09: name = "gt"; break;
                    case 0x0A: name = "wsub"; break;
                    case 0x0B: name = "out"; break;
                    case 0x0C: name = "sub"; break;
                    case 0x0D: name = "startp"; break;
                    case 0x0E: name = "outbyte"; break;
                    case 0x0F: name = "outword"; break;
                    case 0x10: name = "seterr"; break;
                    case 0x12: name = "resetch"; break;
                    case 0x13: name = "csub0"; break;
                    case 0x15: name = "stopp"; break;
                    case 0x16: name = "ladd"; break;
                    case 0x17: name = "stlb"; break;
                    case 0x18: name = "sthf"; break;
                    case 0x19: name = "norm"; break;
                    case 0x1A: name = "ldiv"; break;
                    case 0x1B: name = "ldpi"; break;
                    case 0x1C: name = "stlf"; break;
                    case 0x1D: name = "xdble"; break;
                    case 0x1E: name = "ldpri"; break;
                    case 0x1F: name = "rem"; break;
                    case 0x20:
                        name  = "ret";
                        isRet = true;
                        break;

                    case 0x21: name = "lend"; break;
                    case 0x22: name = "ldtimer"; break;
                    case 0x29: name = "testerr"; break;
                    case 0x2A: name = "testpranal"; break;
                    case 0x2B: name = "tin"; break;
                    case 0x2C: name = "div"; break;
                    case 0x2E: name = "dist"; break;
                    case 0x2F: name = "disc"; break;
                    case 0x30: name = "diss"; break;
                    case 0x31: name = "lmul"; break;
                    case 0x32: name = "not"; break;
                    case 0x33: name = "xor"; break;
                    case 0x34: name = "bcnt"; break;
                    case 0x35: name = "lshr"; break;
                    case 0x36: name = "lshl"; break;
                    case 0x37: name = "lsum"; break;
                    case 0x38: name = "lsub"; break;
                    case 0x39: name = "runp"; break;
                    case 0x3A: name = "xword"; break;
                    case 0x3B: name = "sb"; break;
                    case 0x3C: name = "gajw"; break;
                    case 0x3D: name = "savel"; break;
                    case 0x3E: name = "saveh"; break;
                    case 0x3F: name = "wcnt"; break;
                    case 0x40: name = "shr"; break;
                    case 0x41: name = "shl"; break;
                    case 0x42: name = "mint"; break;
                    case 0x43: name = "alt"; break;
                    case 0x44: name = "altwt"; break;
                    case 0x45: name = "altend"; break;
                    case 0x46: name = "and"; break;
                    case 0x47: name = "enbt"; break;
                    case 0x48: name = "enbc"; break;
                    case 0x49: name = "enbs"; break;
                    case 0x4A: name = "move"; break;
                    case 0x4B: name = "or"; break;
                    case 0x4C: name = "csngl"; break;
                    case 0x4D: name = "ccnt1"; break;
                    case 0x4E: name = "talt"; break;
                    case 0x4F: name = "ldiff"; break;
                    case 0x50: name = "sthb"; break;
                    case 0x51: name = "taltwt"; break;
                    case 0x52: name = "sum"; break;
                    case 0x53: name = "mul"; break;
                    case 0x54: name = "sttimer"; break;
                    case 0x55: name = "stoperr"; break;
                    case 0x56: name = "cword"; break;
                    case 0x57: name = "clrhalterr"; break;
                    case 0x58: name = "sethalterr"; break;
                    case 0x59: name = "testhalterr"; break;
                    case 0x5A: name = "dup"; break;
                    case 0x5B: name = "move2dinit"; break;
                    case 0x5C: name = "move2dall"; break;
                    case 0x5D: name = "move2dnonzero"; break;
                    case 0x5E: name = "move2dzero"; break;
                    case 0x5F: name = "gtu"; break;
                    case 0x63: name = "unpacksn"; break;
                    case 0x64: name = "slmul"; break;
                    case 0x65: name = "sulmul"; break;
                    case 0x68: name = "satadd"; break;
                    case 0x69: name = "satsub"; break;
                    case 0x6A: name = "satmul"; break;
                    case 0x6C: name = "postnormsn"; break;
                    case 0x6D: name = "roundsn"; break;
                    case 0x6E: name = "ldtraph"; break;
                    case 0x6F: name = "sttraph"; break;
                    case 0x71: name = "ldinf"; break;
                    case 0x72: name = "fmul"; break;
                    case 0x73: name = "cflerr"; break;
                    case 0x74: name = "crcword"; break;
                    case 0x75: name = "crcbyte"; break;
                    case 0x76: name = "bitcnt"; break;
                    case 0x77: name = "bitrevword"; break;
                    case 0x78: name = "bitrevnbits"; break;
                    case 0x79: name = "pop"; break;
                    case 0x7E: name = "ldmemstartval"; break;
                    case 0x81: name = "wsubdb"; break;
                    case 0x9C: name = "fptesterr"; break;
                    case 0xB0: name = "settimeslice"; break;
                    case 0xB8: name = "xbword"; break;
                    case 0xB9: name = "lbx"; break;
                    case 0xBA: name = "cb"; break;
                    case 0xBB: name = "cbu"; break;
                    case 0xC1: name = "ssub"; break;
                    case 0xC4: name = "intdis"; break;
                    case 0xC5: name = "intenb"; break;
                    case 0xC6: name = "ldtrapped"; break;
                    case 0xC7: name = "cir"; break;
                    case 0xC8: name = "ss"; break;
                    case 0xCA: name = "ls"; break;
                    case 0xCB: name = "sttrapped"; break;
                    case 0xCC: name = "ciru"; break;
                    case 0xCD: name = "gintdis"; break;
                    case 0xCE: name = "gintenb"; break;
                    case 0xF0: name = "devlb"; break;
                    case 0xF1: name = "devsb"; break;
                    case 0xF2: name = "devls"; break;
                    case 0xF3: name = "devss"; break;
                    case 0xF4: name = "devlw"; break;
                    case 0xF5: name = "devsw"; break;
                    case 0xF6: name = "null"; break;
                    case 0xF7: name = "null"; break;
                    case 0xF8: name = "xsword"; break;
                    case 0xF9: name = "lsx"; break;
                    case 0xFA: name = "cs"; break;
                    case 0xFB: name = "csu"; break;
                    case 0x17C: name = "lddevid"; break;
                    }
                }
                else {
                    // Total is negative, as a result of nfixes

                    total = (~total & ~0xF) | (total & 0xF); // 1's complement the upper nibbles

                    switch (total) {
                    case 0x00: name = "swapqueue"; break;
                    case 0x01: name = "swaptimer"; break;
                    case 0x02: name = "insertqueue"; break;
                    case 0x03: name = "timeslice"; break;
                    case 0x04: name = "signal"; break;
                    case 0x05: name = "wait"; break;
                    case 0x06: name = "trapdis"; break;
                    case 0x07: name = "trapenb"; break;
                    case 0x0B:
                        name  = "tret";
                        isRet = true;
                        break;

                    case 0x0C: name = "ldshadow"; break;
                    case 0x0D: name = "stshadow"; break;
                    case 0x1F:
                        name  = "iret";
                        isRet = true;
                        break;

                    case 0x24: name = "devmove"; break;
                    case 0x2E: name = "restart"; break;
                    case 0x2F: name = "causeerror"; break;
                    case 0x30: name = "nop"; break;
                    case 0x4C: name = "stclock"; break;
                    case 0x4D: name = "ldclock"; break;
                    case 0x4E: name = "clockdis"; break;
                    case 0x4F: name = "clockenb"; break;
                    case 0x8C: name = "ldprodid"; break;
                    case 0x8D: name = "reboot"; break;
                    }
                }

                if (name) {
                    result.rtl = instantiate(pc, name);

                    if (isRet) {
                        result.rtl = std::move(stmts);
                        result.rtl->append(new ReturnStatement);
                    }
                }
                else {
                    result.valid    = false; // Invalid instruction
                    result.rtl      = nullptr;
                    result.numBytes = 0;

                    return false;
                }
            } break;

            default: assert(false);
            }
        }

        break;
    }

    if (result.rtl == nullptr) {
        result.rtl = std::move(stmts);
    }

    return result.valid;
}


ST20Decoder::ST20Decoder(Project *project)
    : NJMCDecoder(project, "ssl/st20.ssl")
{
}


SharedExp ST20Decoder::dis_Reg(int regNum)
{
    return Location::regOf(regNum);
}


void ST20Decoder::processUnconditionalJump(const char *name, int size, HostAddress relocd,
                                           ptrdiff_t delta, Address pc, DecodeResult &result)
{
    result.numBytes     = size;
    GotoStatement *jump = new GotoStatement();
    jump->setDest(Address((relocd - delta).value()));
    result.rtl->append(jump);
    SHOW_ASM(name << " " << relocd - delta)
}


BOOMERANG_DEFINE_PLUGIN(PluginType::Decoder, ST20Decoder, "ST20 decoder plugin", BOOMERANG_VERSION,
                        "Boomerang developers")
