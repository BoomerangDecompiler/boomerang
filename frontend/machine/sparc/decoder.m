/*
 * Copyright (C) 1996-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       decoder.m
 * OVERVIEW:   Implementation of the SPARC specific parts of the
 *             SparcDecoder class.
 *============================================================================*/

/* $Revision$
 *
 * 26 Apr 02 - Mike: Mods for boomerang
 * 19 May 02 - Mike: Added many (int) casts: variables from toolkit are unsgnd
 * 21 May 02 - Mike: SAVE and RESTORE have full semantics now
 * 30 Oct 02 - Mike: dis_Eaddr mode indirectA had extra memof
*/

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "proc.h"
#include "prog.h"
#include "decoder.h"
#include "sparcdecoder.h"
#include "exp.h"
#include "rtl.h"
#include "BinaryFile.h"		// For SymbolByAddress()

#define DIS_ROI     (dis_RegImm(roi))
#define DIS_ADDR    (dis_Eaddr(addr))
// The following are all dis_Num because in the SSL file, we have r[rd],
// and we don't want r[r[14]]!
#define DIS_RD      (dis_Num(rd))
#define DIS_RS1     (dis_Num(rs1))
#define DIS_FS1S    (dis_Num(fs1s+32))
#define DIS_FS2S    (dis_Num(fs2s+32))
// Note: Sparc V9 has a second set of double precision registers that have an
// odd index. So far we only support V8
#define DIS_FDS     (dis_Num((fds>>1)+64))
#define DIS_FS1D    (dis_Num((fs1d>>1)+64))
#define DIS_FS2D    (dis_Num((fs2d>>1)+64))
#define DIS_FDD     (dis_Num((fdd>>1)+64))
#define DIS_FDQ     (dis_Num((fdq>>2)+80))
#define DIS_FS1Q    (dis_Num((fs1q>>2)+80))
#define DIS_FS2Q    (dis_Num((fs2q>>2)+80))


/*==============================================================================
 * FUNCTION:       unused
 * OVERVIEW:       A dummy function to suppress "unused local variable" messages
 * PARAMETERS:     x: integer variable to be "used"
 * RETURNS:        Nothing
 *============================================================================*/
void SparcDecoder::unused(int x)
{}

/*==============================================================================
 * FUNCTION:       createJcond
 * OVERVIEW:       Create an RTL for a Bx instruction
 * PARAMETERS:     pc - the location counter
 *                  exps - ptr to list of Exp pointers
 *                  name - instruction name (e.g. "BNE,a")
 * RETURNS:        Pointer to newly created RTL, or NULL if invalid
 *============================================================================*/
HLJcond* SparcDecoder::createJcond(ADDRESS pc, std::list<Exp*>* exps, const char* name)
{
    HLJcond* res = new HLJcond(pc, exps);
    if (name[0] == 'F') {
        // fbranch is any of [ FBN FBNE FBLG FBUL FBL   FBUG FBG   FBU
        //                     FBA FBE  FBUE FBGE FBUGE FBLE FBULE FBO ],
        // fbranches are not the same as ibranches, so need a whole different
        // set of tests
        if (name[2] == 'U')
            name++;             // Just ignore unordered (for now)
        switch (name[2]) {
        case 'E':                           // FBE
            res->setCondType(HLJCOND_JE, true);
            break;
        case 'L':
            if (name[3] == 'G')             // FBLG
                res->setCondType(HLJCOND_JNE, true);
            else if (name[3] == 'E')        // FBLE
                res->setCondType(HLJCOND_JSLE, true);
            else                            // FBL
                res->setCondType(HLJCOND_JSL, true);
            break;
        case 'G':
            if (name[3] == 'E')             // FBGE
                res->setCondType(HLJCOND_JSGE, true);
            else                            // FBG
                res->setCondType(HLJCOND_JSG, true);
            break;
        case 'N':
            if (name[3] == 'E')             // FBNE
                res->setCondType(HLJCOND_JNE, true);
            // Else it's FBN!
            break;
        default:
            std::cerr << "unknown float branch " << name << std::endl;
            delete res;
            res = NULL;
        }
        return res;
    }   

    // ibranch is any of [ BN BE  BLE BL  BLEU BCS BNEG BVS
    //                     BA BNE BG  BGE BGU  BCC BPOS BVC ],
    switch(name[1]) {
    case 'E':
        res->setCondType(HLJCOND_JE);           // BE
        break;
    case 'L':
        if (name[2] == 'E') {
            if (name[3] == 'U')
                res->setCondType(HLJCOND_JULE); // BLEU
            else
                res->setCondType(HLJCOND_JSLE); // BLE
        }
        else
            res->setCondType(HLJCOND_JSL);      // BL
        break;
    case 'N':
        // BNE, BNEG (won't see BN)
        if (name[3] == 'G')
            res->setCondType(HLJCOND_JMI);      // BNEG
        else
            res->setCondType(HLJCOND_JNE);      // BNE
        break;
    case 'C':
        // BCC, BCS
        if (name[2] == 'C')
            res->setCondType(HLJCOND_JUGE);     // BCC
        else
            res->setCondType(HLJCOND_JUL);      // BCS
        break;
    case 'V':
        // BVC, BVS; should never see these now
        if (name[2] == 'C')
            std::cerr << "Decoded BVC instruction\n";   // BVC
        else
            std::cerr << "Decoded BVS instruction\n";   // BVS
        break;
    case 'G':   
        // BGE, BG, BGU
        if (name[2] == 'E')
            res->setCondType(HLJCOND_JSGE);     // BGE
        else if (name[2] == 'U')
            res->setCondType(HLJCOND_JUG);      // BGU
        else
            res->setCondType(HLJCOND_JSG);      // BG
        break;
    case 'P':   
        res->setCondType(HLJCOND_JPOS);         // BG
        break;
    default:
        std::cerr << "unknown non-float branch " << name << std::endl;
    }   
    return res;
}


/*==============================================================================
 * FUNCTION:       SparcDecoder::decodeInstruction
 * OVERVIEW:       Attempt to decode the high level instruction at a given
 *                 address and return the corresponding HL type (e.g. HLCall,
 *                 HLJump etc). If no high level instruction exists at the
 *                 given address, then simply return the RTL for the low level
 *                 instruction at this address. There is an option to also
 *                 include the exps for a HL instruction.
 * PARAMETERS:     pc - the native address of the pc
 *                 delta - the difference between the above address and the
 *                   host address of the pc (i.e. the address that the pc is at
 *                   in the loaded object file)
 *                 proc - the enclosing procedure. This can be NULL for
 *                   those of us who are using this method in an interpreter
 * RETURNS:        a DecodeResult structure containing all the information
 *                   gathered during decoding
 *============================================================================*/
DecodeResult& SparcDecoder::decodeInstruction (ADDRESS pc, int delta)
{ 
    static DecodeResult result;
    ADDRESS hostPC = pc+delta;

    // Clear the result structure;
    result.reset();

    // The actual list of instantiated exps
    std::list<Exp*>* exps = NULL;

    ADDRESS nextPC;

    match [nextPC] hostPC to

    | call__(addr) =>
        /*
         * A standard call 
         */
        HLCall* newCall = new HLCall(pc, 0, 0);

        // Set the destination
        newCall->setDest(addr - delta);
        result.rtl = newCall;
        result.type = SD;
        SHOW_ASM("call__ ")

    | call_(addr) =>
        /*
         * A jmpl with rd == %o7, i.e. a register call
         */
        HLCall* newCall = new HLCall(pc, 0, 0);

        // Record the fact that this is a computed call
        newCall->setIsComputed();

        // Set the destination expression
        newCall->setDest(dis_Eaddr(addr));
        result.rtl = newCall;
        result.type = DD;
        SHOW_ASM("call_ ")


    | ret() =>
        /*
         * Just a ret, no restore
         */
        result.rtl = new HLReturn(pc, exps);
        result.type = DD;
        SHOW_ASM("ret_")

    | branch^",a" (tgt) [name] => 
        /*
         * Anulled branch
         */

        // First, check for CBxxx branches (branches that depend on
        // co-processor instructions). These are invalid, as far as
        // we are concerned
        if (name[0] == 'C') {
            result.valid = false;
            result.rtl = new RTL;
            result.numBytes = 4;
            return result;
        }
        // Instantiate a HLJump for the unconditional branches,
        // HLJconds for the rest.
        // NOTE: NJMC toolkit cannot handle embedded else statements!
        HLJump* jump = 0;
        if (strcmp(name,"BA,a") == 0 || strcmp(name,"BN,a") == 0)
            jump = new HLJump(pc, exps);
        if ((jump == 0) &&
          (strcmp(name,"BVS,a") == 0 || strcmp(name,"BVC,a") == 0))
            jump = new HLJump(pc, exps);
        if (jump == 0)
            jump = createJcond(pc, exps, name);

        if (jump == NULL) {
            result.valid = false;
            result.rtl = new RTL;
            result.numBytes = 4;
            return result;
        }

        // The class of this instruction depends on whether or not
        // it is one of the 'unconditional' conditional branches
        // "BA,A" or "BN,A"
        result.type = SCDAN;
        if ((strcmp(name,"BA,a") == 0) || (strcmp(name, "BVC,a") == 0))
            result.type = SU;
        if ((strcmp(name,"BN,a") == 0) || (strcmp(name, "BVS,a") == 0))
            result.type = SKIP;

        result.rtl = jump;
        jump->setDest(tgt - delta);
        SHOW_ASM(name << " " << hex << tgt-delta)
        
    | branch (tgt) [name] => 
        /*
         * Non anulled branch
         */
        // First, check for CBxxx branches (branches that depend on
        // co-processor instructions). These are invalid, as far as
        // we are concerned
        if (name[0] == 'C') {
            result.valid = false;
            result.rtl = new RTL;
            result.numBytes = 4;
            return result;
        }
        // Instantiate a HLJump for the unconditional branches,
        // HLJconds for the rest
        // NOTE: NJMC toolkit cannot handle embedded else statements!
        HLJump* jump = 0;
        if (strcmp(name,"BA") == 0 || strcmp(name,"BN") == 0)
            jump = new HLJump(pc, exps);
        if ((jump == 0) &&
          (strcmp(name,"BVS") == 0 || strcmp(name,"BVC") == 0))
            jump = new HLJump(pc, exps);
        if (jump == 0)
            jump = createJcond(pc, exps, name);

        // The class of this instruction depends on whether or not
        // it is one of the 'unconditional' conditional branches
        // "BA" or "BN" (or the pseudo unconditionals BVx)
        result.type = SCD;
        if ((strcmp(name,"BA") == 0) || (strcmp(name, "BVC") == 0))
            result.type = SD;
        if ((strcmp(name,"BN") == 0) || (strcmp(name, "BVS") == 0))
            result.type = NCT;

        result.rtl = jump;
        jump->setDest(tgt - delta);
        SHOW_ASM(name << " " << hex << tgt-delta)


    | JMPL (addr, rd) =>
        /*
         * JMPL, with rd != %o7, i.e. register jump
         */
        HLNwayJump* jump = new HLNwayJump(pc, exps);
        // Record the fact that it is a computed jump
        jump->setIsComputed();
        result.rtl = jump;
        result.type = DD;
        jump->setDest(dis_Eaddr(addr));
        unused(rd);
        SHOW_ASM("JMPL ")
#if DEBUG_DECODER
        jump->getDest()->print();
#endif


    //  //  //  //  //  //  //  //
    //                          //
    //   Ordinary instructions  //
    //                          //
    //  //  //  //  //  //  //  //

    | SAVE (rs1, roi, rd) =>
        // Decided to treat SAVE as an ordinary instruction
        // That is, use the large list of effects from the SSL file, and
        // hope that optimisation will vastly help the common cases
        exps = instantiate(pc, "SAVE", DIS_RS1, DIS_ROI, DIS_RD);

    | RESTORE (rs1, roi, rd) =>
        // Decided to treat RESTORE as an ordinary instruction
        exps = instantiate(pc, "RESTORE", DIS_RS1, DIS_ROI, DIS_RD);

	| NOP [name] =>
		result.type = NOP;
		exps = instantiate(pc,  name);

	| sethi(imm22, rd) => 
		exps = instantiate(pc,  "sethi", dis_Num(imm22), DIS_RD);

	| load_greg(addr, rd) [name] => 
		exps = instantiate(pc,  name, DIS_ADDR, DIS_RD);

	| LDF (addr, fds) [name] => 
		exps = instantiate(pc,  name, DIS_ADDR, DIS_FDS);

	| LDDF (addr, fdd) [name] => 
		exps = instantiate(pc,  name, DIS_ADDR, DIS_FDD);

	| load_asi (addr, asi, rd) [name] => 
        unused(asi);            // Note: this could be serious!
		exps = instantiate(pc,  name, DIS_RD, DIS_ADDR);

	| sto_greg(rd, addr) [name] => 
		exps = instantiate(pc,  name, DIS_RD, DIS_ADDR);

	| STF (fds, addr) [name] => 
		exps = instantiate(pc,  name, DIS_FDS, DIS_ADDR);

	| STDF (fdd, addr) [name] => 
		exps = instantiate(pc,  name, DIS_FDD, DIS_ADDR);

	| sto_asi (rd, addr, asi) [name] => 
        unused(asi);            // Note: this could be serious!
		exps = instantiate(pc,  name, DIS_RD, DIS_ADDR);

	| LDFSR(addr) [name] => 
		exps = instantiate(pc,  name, DIS_ADDR);

	| LDCSR(addr) [name] => 
		exps = instantiate(pc,  name, DIS_ADDR);

	| STFSR(addr) [name] => 
		exps = instantiate(pc,  name, DIS_ADDR);

	| STCSR(addr) [name] => 
		exps = instantiate(pc,  name, DIS_ADDR);

	| STDFQ(addr) [name] => 
		exps = instantiate(pc,  name, DIS_ADDR);

	| STDCQ(addr) [name] => 
		exps = instantiate(pc,  name, DIS_ADDR);

	| RDY(rd) [name] => 
		exps = instantiate(pc,  name, DIS_RD);

	| RDPSR(rd) [name] => 
		exps = instantiate(pc,  name, DIS_RD);

	| RDWIM(rd) [name] => 
		exps = instantiate(pc,  name, DIS_RD);

	| RDTBR(rd) [name]	=> 
		exps = instantiate(pc,  name, DIS_RD);

	| WRY(rs1,roi) [name]	=> 
		exps = instantiate(pc,  name, DIS_RS1, DIS_ROI);

	| WRPSR(rs1, roi) [name] => 
		exps = instantiate(pc,  name, DIS_RS1, DIS_ROI);

	| WRWIM(rs1, roi) [name] => 
		exps = instantiate(pc,  name, DIS_RS1, DIS_ROI);

	| WRTBR(rs1, roi) [name] => 
		exps = instantiate(pc,  name, DIS_RS1, DIS_ROI);

	| alu (rs1, roi, rd) [name] => 
		exps = instantiate(pc,  name, DIS_RS1, DIS_ROI, DIS_RD);

	| float2s (fs2s, fds) [name] => 
		exps = instantiate(pc,  name, DIS_FS2S, DIS_FDS);

	| float3s (fs1s, fs2s, fds) [name] => 
		exps = instantiate(pc,  name, DIS_FS1S, DIS_FS2S, DIS_FDS);
 
	| float3d (fs1d, fs2d, fdd) [name] => 
		exps = instantiate(pc,  name, DIS_FS1D, DIS_FS2D, DIS_FDD);
 
	| float3q (fs1q, fs2q, fdq) [name] => 
		exps = instantiate(pc,  name, DIS_FS1Q, DIS_FS2Q, DIS_FDQ);
 
	| fcompares (fs1s, fs2s) [name] => 
		exps = instantiate(pc,  name, DIS_FS1S, DIS_FS2S);

	| fcompared (fs1d, fs2d) [name] => 
		exps = instantiate(pc,  name, DIS_FS1D, DIS_FS2D);

	| fcompareq (fs1q, fs2q) [name] => 
		exps = instantiate(pc,  name, DIS_FS1Q, DIS_FS2Q);

    | FTOs (fs2s, fds) [name] =>
        exps = instantiate(pc, name, DIS_FS2S, DIS_FDS);

    // Note: itod and dtoi have different sized registers
    | FiTOd (fs2s, fdd) [name] =>
        exps = instantiate(pc, name, DIS_FS2S, DIS_FDD);
    | FdTOi (fs2d, fds) [name] =>
        exps = instantiate(pc, name, DIS_FS2D, DIS_FDS);

    | FiTOq (fs2s, fdq) [name] =>
        exps = instantiate(pc, name, DIS_FS2S, DIS_FDQ);
    | FqTOi (fs2q, fds) [name] =>
        exps = instantiate(pc, name, DIS_FS2Q, DIS_FDS);

    | FsTOd (fs2s, fdd) [name] =>
        exps = instantiate(pc, name, DIS_FS2S, DIS_FDD);
    | FdTOs (fs2d, fds) [name] =>
        exps = instantiate(pc, name, DIS_FS2D, DIS_FDS);

    | FsTOq (fs2s, fdq) [name] =>
        exps = instantiate(pc, name, DIS_FS2S, DIS_FDQ);
    | FqTOs (fs2q, fds) [name] =>
        exps = instantiate(pc, name, DIS_FS2Q, DIS_FDS);

    | FdTOq (fs2d, fdq) [name] =>
        exps = instantiate(pc, name, DIS_FS2D, DIS_FDQ);
    | FqTOd (fs2q, fdd) [name] =>
        exps = instantiate(pc, name, DIS_FS2Q, DIS_FDD);


    | FSQRTd (fs2d, fdd) [name] =>
        exps = instantiate(pc, name, DIS_FS2D, DIS_FDD);

    | FSQRTq (fs2q, fdq) [name] =>
        exps = instantiate(pc, name, DIS_FS2Q, DIS_FDQ);


	| JMPL (addr, rd) [name] => 
		result.type = DD;
		exps = instantiate(pc,  name, DIS_ADDR, DIS_RD);

	| RETT (addr) [name] => 
        unused(addr);
		exps = instantiate(pc,  name);

	| trap (addr) [name] => 
		exps = instantiate(pc,  name, DIS_ADDR);

	| UNIMP (n) => 
        unused(n);
		exps = NULL;
        result.valid = false;

	| inst = n => 
        // What does this mean?
        unused(n);
        result.valid = false;
		exps = NULL;

    else
		exps = NULL;
        result.valid = false;
    endmatch

    result.numBytes = nextPC - hostPC;
    if (result.valid && result.rtl == 0)    // Don't override higher level res
        result.rtl = new RTL(pc, exps);

    return result;
}


/***********************************************************************
 * These are functions used to decode instruction operands into
 * expressions (Exp*s).
 **********************************************************************/

/*==============================================================================
 * FUNCTION:        SparcDecoder::dis_RegImm
 * OVERVIEW:        Decode the register or immediate at the given
 *                  address.
 * NOTE:            Used via macro DIS_ROI
 * PARAMETERS:      pc - an address in the instruction stream
 * RETURNS:         the register or immediate at the given address
 *============================================================================*/
Exp* SparcDecoder::dis_RegImm(unsigned pc)
{

    match pc to
    | imode(i) =>
        Exp* expr = new Const(i);
        return expr;
    | rmode(rs2) =>
        Exp* expr = new Unary(opRegOf, new Const((int) rs2));
        return expr;
    endmatch
}

/*==============================================================================
 * FUNCTION:        SparcDecoder::dis_Eaddr
 * OVERVIEW:        Converts a dynamic address to a Exp* expression.
 *                  E.g. %o7 --> r[ 15 ]
 * PARAMETERS:      pc - the instruction stream address of the dynamic
 *                    address
 *                  ignore - redundant parameter on SPARC
 * RETURNS:         the Exp* representation of the given address
 *============================================================================*/
Exp* SparcDecoder::dis_Eaddr(ADDRESS pc, int ignore /* = 0 */)
{
    Exp* expr;

    match pc to
    | indirectA(rs1) =>
        expr = new Unary(opRegOf, new Const((int)rs1));
    | indexA(rs1, rs2) =>
        expr = new Binary(opPlus,
            new Unary(opRegOf, new Const((int)rs1)),
            new Unary(opRegOf, new Const((int)rs2)));
    | absoluteA(i) =>
        expr = new Const((int)i);
    | dispA(rs1,i) =>
        expr = new Binary(opPlus,
            new Unary(opRegOf, new Const((int)rs1)),
            new Const((int)i));
    endmatch

    return expr;
}

/*==============================================================================
 * FUNCTION:      isFuncPrologue()
 * OVERVIEW:      Check to see if the instructions at the given offset match
 *                  any callee prologue, i.e. does it look like this offset
 *                  is a pointer to a function?
 * PARAMETERS:    hostPC - pointer to the code in question (host address)
 * RETURNS:       True if a match found
 *============================================================================*/
bool SparcDecoder::isFuncPrologue(ADDRESS hostPC)
{
#if 0       // Can't do this without patterns. It was a bit of a hack anyway
    int hiVal, loVal, reg, locals;
    if ((InstructionPatterns::new_reg_win(prog.csrSrc,hostPC, locals)) != NULL)
            return true;
    if ((InstructionPatterns::new_reg_win_large(prog.csrSrc, hostPC,
        hiVal, loVal, reg)) != NULL)
            return true;
    if ((InstructionPatterns::same_reg_win(prog.csrSrc, hostPC, locals))
        != NULL)
            return true;
    if ((InstructionPatterns::same_reg_win_large(prog.csrSrc, hostPC,
        hiVal, loVal, reg)) != NULL)
            return true;
#endif

    return false;
}

/*==============================================================================
 * FUNCTION:      isRestore()
 * OVERVIEW:      Check to see if the instruction at the given offset is a
 *                  restore instruction
 * PARAMETERS:    hostPC - pointer to the code in question (host address)
 * RETURNS:       True if a match found
 *============================================================================*/
bool SparcDecoder::isRestore(ADDRESS hostPC) {
        match hostPC to
        | RESTORE(a, b, c) =>
            unused(a);      // Suppress warning messages
            unused(b);
            unused(c);
            return true;
        else
            return false;
        endmatch
}

 /**********************************
 * These are the fetch routines.
 **********************************/

/*==============================================================================
 * FUNCTION:        getDword
 * OVERVIEW:        Returns the double starting at the given address.
 * PARAMETERS:      lc - address at which to decode the double
 * RETURNS:         the decoded double
 *============================================================================*/
DWord SparcDecoder::getDword(ADDRESS lc)
{
  Byte* p = (Byte*)lc;
  return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}

/*==============================================================================
 * FUNCTION:       SparcDecoder::SparcDecoder
 * OVERVIEW:       
 * PARAMETERS:     None
 * RETURNS:        N/A
 *============================================================================*/
SparcDecoder::SparcDecoder() : NJMCDecoder()
{}

// For now...
int SparcDecoder::decodeAssemblyInstruction(unsigned, int)
{ return 0; }

