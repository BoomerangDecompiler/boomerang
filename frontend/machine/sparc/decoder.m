/*
 * Copyright (C) 1996-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   decoder.m
 * OVERVIEW:   Implementation of the SPARC specific parts of the
 *			   SparcDecoder class.
 *============================================================================*/

/* $Revision$	// 1.20.2.2
 *
 * 26 Apr 02 - Mike: Mods for boomerang
 * 19 May 02 - Mike: Added many (int) casts: variables from toolkit are unsgnd
 * 21 May 02 - Mike: SAVE and RESTORE have full semantics now
 * 30 Oct 02 - Mike: dis_Eaddr mode indirectA had extra memof
 * 22 Nov 02 - Mike: Support 32 bit V9 branches
 * 04 Dec 02 - Mike: r[0] -> 0 automatically (rhs only)
 * 30 May 02 - Mike: Also fixed r[0] -> 0 for store instructions
 * 03 Nov 04 - Mike: DIS_FDS was returning numbers for the double precision registers
*/

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#include <cstring>
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"
#endif

#include "decoder.h"
#include "exp.h"
#include "prog.h"
#include "proc.h"
#include "sparcdecoder.h"
#include "rtl.h"
#include "BinaryFile.h"		// For SymbolByAddress()
#include "boomerang.h"

#define DIS_ROI		(dis_RegImm(roi))
#define DIS_ADDR	(dis_Eaddr(addr))
#define DIS_RD		(dis_RegLhs(rd))
#define DIS_RDR		(dis_RegRhs(rd))
#define DIS_RS1		(dis_RegRhs(rs1))
#define DIS_FS1S	(dis_RegRhs(fs1s+32))
#define DIS_FS2S	(dis_RegRhs(fs2s+32))
// Note: Sparc V9 has a second set of double precision registers that have an
// odd index. So far we only support V8
#define DIS_FDS		(dis_RegLhs(fds+32))
#define DIS_FS1D	(dis_RegRhs((fs1d>>1)+64))
#define DIS_FS2D	(dis_RegRhs((fs2d>>1)+64))
#define DIS_FDD		(dis_RegLhs((fdd>>1)+64))
#define DIS_FDQ		(dis_RegLhs((fdq>>2)+80))
#define DIS_FS1Q	(dis_RegRhs((fs1q>>2)+80))
#define DIS_FS2Q	(dis_RegRhs((fs2q>>2)+80))

/*==============================================================================
 * FUNCTION:	   unused
 * OVERVIEW:	   A dummy function to suppress "unused local variable" messages
 * PARAMETERS:	   x: integer variable to be "used"
 * RETURNS:		   Nothing
 *============================================================================*/
void SparcDecoder::unused(int x)
{}

/*==============================================================================
 * FUNCTION:	   createBranchRtl
 * OVERVIEW:	   Create an RTL for a Bx instruction
 * PARAMETERS:	   pc - the location counter
 *				   stmts - ptr to list of Statement pointers
 *				   name - instruction name (e.g. "BNE,a", or "BPNE")
 * RETURNS:		   Pointer to newly created RTL, or NULL if invalid
 *============================================================================*/
RTL* SparcDecoder::createBranchRtl(ADDRESS pc, std::list<Statement*>* stmts, const char* name) {
	RTL* res = new RTL(pc, stmts);
	BranchStatement* br = new BranchStatement();
	res->appendStmt(br);
	if (name[0] == 'F') {
		// fbranch is any of [ FBN FBNE FBLG FBUL FBL	FBUG FBG   FBU
		//					   FBA FBE	FBUE FBGE FBUGE FBLE FBULE FBO ],
		// fbranches are not the same as ibranches, so need a whole different set of tests
		if (name[2] == 'U')
			name++;				// Just ignore unordered (for now)
		switch (name[2]) {
		case 'E':							// FBE
			br->setCondType(BRANCH_JE, true);
			break;
		case 'L':
			if (name[3] == 'G')				// FBLG
				br->setCondType(BRANCH_JNE, true);
			else if (name[3] == 'E')		// FBLE
				br->setCondType(BRANCH_JSLE, true);
			else							// FBL
				br->setCondType(BRANCH_JSL, true);
			break;
		case 'G':
			if (name[3] == 'E')				// FBGE
				br->setCondType(BRANCH_JSGE, true);
			else							// FBG
				br->setCondType(BRANCH_JSG, true);
			break;
		case 'N':
			if (name[3] == 'E')				// FBNE
				br->setCondType(BRANCH_JNE, true);
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
	//					   BA BNE BG  BGE BGU  BCC BPOS BVC ],
	// Note: BPN, BPE, etc handled below
	switch(name[1]) {
	case 'E':
		br->setCondType(BRANCH_JE);			  // BE
		break;
	case 'L':
		if (name[2] == 'E') {
			if (name[3] == 'U')
				br->setCondType(BRANCH_JULE); // BLEU
			else
				br->setCondType(BRANCH_JSLE); // BLE
		}
		else
			br->setCondType(BRANCH_JSL);	  // BL
		break;
	case 'N':
		// BNE, BNEG (won't see BN)
		if (name[3] == 'G')
			br->setCondType(BRANCH_JMI);	  // BNEG
		else
			br->setCondType(BRANCH_JNE);	  // BNE
		break;
	case 'C':
		// BCC, BCS
		if (name[2] == 'C')
			br->setCondType(BRANCH_JUGE);	  // BCC
		else
			br->setCondType(BRANCH_JUL);	  // BCS
		break;
	case 'V':
		// BVC, BVS; should never see these now
		if (name[2] == 'C')
			std::cerr << "Decoded BVC instruction\n";	// BVC
		else
			std::cerr << "Decoded BVS instruction\n";	// BVS
		break;
	case 'G':	
		// BGE, BG, BGU
		if (name[2] == 'E')
			br->setCondType(BRANCH_JSGE);	  // BGE
		else if (name[2] == 'U')
			br->setCondType(BRANCH_JUG);	  // BGU
		else
			br->setCondType(BRANCH_JSG);	  // BG
		break;
	case 'P':	
		if (name[2] == 'O') {
			br->setCondType(BRANCH_JPOS);		  // BPOS
			break;
		}
		// Else, it's a BPXX; remove the P (for predicted) and try again
		// (recurse)
		// B P P O S ...
		// 0 1 2 3 4 ...
		char temp[8];
		temp[0] = 'B';
		strcpy(temp+1, name+2);
		delete res;
		return createBranchRtl(pc, stmts, temp);
	default:
		std::cerr << "unknown non-float branch " << name << std::endl;
	}	
	return res;
}


/*==============================================================================
 * FUNCTION:	   SparcDecoder::decodeInstruction
 * OVERVIEW:	   Attempt to decode the high level instruction at a given address and return the corresponding HL type
 *					(e.g. CallStatement, GotoStatement etc). If no high level instruction exists at the given address,
 *					then simply return the RTL for the low level instruction at this address. There is an option to also
 *				   include the low level statements for a HL instruction.
 * PARAMETERS:	   pc - the native address of the pc
 *				   delta - the difference between the above address and the host address of the pc (i.e. the address
 *					that the pc is at in the loaded object file)
 *				   proc - the enclosing procedure. This can be NULL for those of us who are using this method in an
 *					interpreter
 * RETURNS:		   a DecodeResult structure containing all the information gathered during decoding
 *============================================================================*/
DecodeResult& SparcDecoder::decodeInstruction (ADDRESS pc, int delta) { 
	static DecodeResult result;
	ADDRESS hostPC = pc+delta;

	// Clear the result structure;
	result.reset();

	// The actual list of instantiated statements
	std::list<Statement*>* stmts = NULL;

	ADDRESS nextPC = NO_ADDRESS;

	match [nextPC] hostPC to

	| call__(addr) =>
		/*
		 * A standard call 
		 */
		CallStatement* newCall = new CallStatement;

		// Set the destination
		ADDRESS nativeDest = addr - delta;
		newCall->setDest(nativeDest);
		Proc* destProc = prog->setNewProc(nativeDest);
		if (destProc == (Proc*)-1) destProc = NULL;
		newCall->setDestProc(destProc);
		result.rtl = new RTL(pc, stmts);
		result.rtl->appendStmt(newCall);
		result.type = SD;
		SHOW_ASM("call__ " << std::hex << (nativeDest))
		DEBUG_STMTS

	| call_(addr) =>
		/*
		 * A JMPL with rd == %o7, i.e. a register call
		 */
		CallStatement* newCall = new CallStatement;

		// Record the fact that this is a computed call
		newCall->setIsComputed();

		// Set the destination expression
		newCall->setDest(dis_Eaddr(addr));
		result.rtl = new RTL(pc, stmts);
		result.rtl->appendStmt(newCall);
		result.type = DD;

		SHOW_ASM("call_ " << dis_Eaddr(addr))
		DEBUG_STMTS


	| ret() =>
		/*
		 * Just a ret (non leaf)
		 */
		result.rtl = new RTL(pc, stmts);
		result.rtl->appendStmt(new ReturnStatement);
		result.type = DD;
		SHOW_ASM("ret_")
		DEBUG_STMTS

	| retl() =>
		/*
		 * Just a ret (leaf; uses %o7 instead of %i7)
		 */
		result.rtl = new RTL(pc, stmts);
		result.rtl->appendStmt(new ReturnStatement);
		result.type = DD;
		SHOW_ASM("retl_")
		DEBUG_STMTS

	| branch^",a" (tgt) [name] => 
		/*
		 * Anulled branch
		 */

		// First, check for CBxxx branches (branches that depend on co-processor instructions). These are invalid,
		// as far as we are concerned
		if (name[0] == 'C') {
			result.valid = false;
			result.rtl = new RTL;
			result.numBytes = 4;
			return result;
		}
		// Instantiate a GotoStatement for the unconditional branches, HLJconds for the rest.
		// NOTE: NJMC toolkit cannot handle embedded else statements!
		GotoStatement* jump = 0;
		RTL* rtl = NULL;					// Init to NULL to suppress a warning
		if (strcmp(name,"BA,a") == 0 || strcmp(name,"BN,a") == 0) {
			jump = new GotoStatement;
			rtl = new RTL(pc, stmts);
			rtl->appendStmt(jump);
		} else if (strcmp(name,"BVS,a") == 0 || strcmp(name,"BVC,a") == 0) {
			jump = new GotoStatement;
			rtl = new RTL(pc, stmts);
			rtl->appendStmt(jump);
		} else {
			rtl = createBranchRtl(pc, stmts, name);
			jump = (GotoStatement*) rtl->getList().back();
		}

		// The class of this instruction depends on whether or not it is one of the 'unconditional' conditional branches
		// "BA,A" or "BN,A"
		result.type = SCDAN;
		if ((strcmp(name,"BA,a") == 0) || (strcmp(name, "BVC,a") == 0)) {
			result.type = SU;
		} else {
			result.type = SKIP;
		}

		result.rtl = rtl;
		jump->setDest(tgt - delta);
		SHOW_ASM(name << " " << std::hex << tgt-delta)
		DEBUG_STMTS
		
	| pbranch^",a" (cc01, tgt) [name] => 
		/*
		 * Anulled , predicted branch (treat as for non predicted)
		 */

		// Instantiate a GotoStatement for the unconditional branches, HLJconds for the rest.
		// NOTE: NJMC toolkit cannot handle embedded else statements!
		if (cc01 != 0) {		/* If 64 bit cc used, can't handle */
			result.valid = false;
			result.rtl = new RTL;
			result.numBytes = 4;
			return result;
		}
		GotoStatement* jump = 0;
		RTL* rtl = NULL;					// Init to NULL to suppress a warning
		if (strcmp(name,"BPA,a") == 0 || strcmp(name,"BPN,a") == 0) {
			jump = new GotoStatement;
			rtl = new RTL(pc, stmts);
			rtl->appendStmt(jump);
		} else if (strcmp(name,"BPVS,a") == 0 || strcmp(name,"BPVC,a") == 0) {
			jump = new GotoStatement;
			rtl = new RTL(pc, stmts);
			rtl->appendStmt(jump);
		} else {
			rtl = createBranchRtl(pc, stmts, name);
			jump = (GotoStatement*) rtl->getList().back();
		}

		// The class of this instruction depends on whether or not it is one of the 'unconditional' conditional branches
		// "BPA,A" or "BPN,A"
		result.type = SCDAN;
		if ((strcmp(name,"BPA,a") == 0) || (strcmp(name, "BPVC,a") == 0)) {
			result.type = SU;
		} else {
			result.type = SKIP;
		}

		result.rtl = rtl;
		jump->setDest(tgt - delta);
		SHOW_ASM(name << " " << std::hex << tgt-delta)
		DEBUG_STMTS
		
	| branch (tgt) [name] => 
		/*
		 * Non anulled branch
		 */
		// First, check for CBxxx branches (branches that depend on co-processor instructions). These are invalid,
		// as far as we are concerned
		if (name[0] == 'C') {
			result.valid = false;
			result.rtl = new RTL;
			result.numBytes = 4;
			return result;
		}
		// Instantiate a GotoStatement for the unconditional branches, BranchStatement for the rest
		// NOTE: NJMC toolkit cannot handle embedded plain else statements! (But OK with curly bracket before the else)
		GotoStatement* jump = 0;
		RTL* rtl = NULL;
		if (strcmp(name,"BA") == 0 || strcmp(name,"BN") == 0) {
			jump = new GotoStatement;
			rtl = new RTL(pc, stmts);
			rtl->appendStmt(jump);
		} else if (strcmp(name,"BVS") == 0 || strcmp(name,"BVC") == 0) {
			jump = new GotoStatement;
			rtl = new RTL(pc, stmts);
			rtl->appendStmt(jump);
		} else {
			rtl = createBranchRtl(pc, stmts, name);
			jump = (BranchStatement*) rtl->getList().back();
		}

		// The class of this instruction depends on whether or not it is one of the 'unconditional' conditional branches
		// "BA" or "BN" (or the pseudo unconditionals BVx)
		result.type = SCD;
		if ((strcmp(name,"BA") == 0) || (strcmp(name, "BVC") == 0))
			result.type = SD;
		if ((strcmp(name,"BN") == 0) || (strcmp(name, "BVS") == 0))
			result.type = NCT;

		result.rtl = rtl;
		jump->setDest(tgt - delta);
		SHOW_ASM(name << " " << std::hex << tgt-delta)
		DEBUG_STMTS

	| BPA (cc01, tgt) =>			/* Can see bpa xcc,tgt in 32 bit code */
		unused(cc01);				// Does not matter because is unconditional
		GotoStatement* jump = new GotoStatement;

		result.type = SD;
		result.rtl = new RTL(pc, stmts);
		result.rtl->appendStmt(jump);
		jump->setDest(tgt - delta);
		SHOW_ASM("BPA " << std::hex << tgt-delta)
		DEBUG_STMTS

	| pbranch (cc01, tgt) [name] =>
		if (cc01 != 0) {		/* If 64 bit cc used, can't handle */
			result.valid = false;
			result.rtl = new RTL;
			result.numBytes = 4;
			return result;
		}
		GotoStatement* jump = 0;
		RTL* rtl = NULL;
		if (strcmp(name,"BPN") == 0) {
			jump = new GotoStatement;
			rtl = new RTL(pc, stmts);
			rtl->appendStmt(jump);
		} else if (strcmp(name,"BPVS") == 0 || strcmp(name,"BPVC") == 0) {
			jump = new GotoStatement;
			rtl = new RTL(pc, stmts);
			rtl->appendStmt(jump);
		} else {
			rtl = createBranchRtl(pc, stmts, name);
			// The BranchStatement will be the last Stmt of the rtl
			jump = (GotoStatement*)rtl->getList().back();
		}

		// The class of this instruction depends on whether or not
		// it is one of the 'unconditional' conditional branches
		// "BPN" (or the pseudo unconditionals BPVx)
		result.type = SCD;
		if (strcmp(name, "BPVC") == 0)
			result.type = SD;
		if ((strcmp(name,"BPN") == 0) || (strcmp(name, "BPVS") == 0))
			result.type = NCT;

		result.rtl = rtl;
		jump->setDest(tgt - delta);
		SHOW_ASM(name << " " << std::hex << tgt-delta)
		DEBUG_STMTS

	| JMPL (addr, rd) =>
		/*
		 * JMPL, with rd != %o7, i.e. register jump
		 * Note: if rd==%o7, then would be handled with the call_ arm
		 */
		CaseStatement* jump = new CaseStatement;
		// Record the fact that it is a computed jump
		jump->setIsComputed();
		result.rtl = new RTL(pc, stmts);
		result.rtl->appendStmt(jump);
		result.type = DD;
		jump->setDest(dis_Eaddr(addr));
		unused(rd);
		SHOW_ASM("JMPL ")
		DEBUG_STMTS


	//	//	//	//	//	//	//	//
	//							//
	//	 Ordinary instructions	//
	//							//
	//	//	//	//	//	//	//	//

	| SAVE (rs1, roi, rd) =>
		// Decided to treat SAVE as an ordinary instruction
		// That is, use the large list of effects from the SSL file, and
		// hope that optimisation will vastly help the common cases
		stmts = instantiate(pc, "SAVE", DIS_RS1, DIS_ROI, DIS_RD);

	| RESTORE (rs1, roi, rd) =>
		// Decided to treat RESTORE as an ordinary instruction
		stmts = instantiate(pc, "RESTORE", DIS_RS1, DIS_ROI, DIS_RD);

	| NOP [name] =>
		result.type = NOP;
		stmts = instantiate(pc,	 name);

	| sethi(imm22, rd) => 
		stmts = instantiate(pc,	 "sethi", dis_Num(imm22), DIS_RD);

	| load_greg(addr, rd) [name] => 
		stmts = instantiate(pc,	 name, DIS_ADDR, DIS_RD);

	| LDF (addr, fds) [name] => 
		stmts = instantiate(pc,	 name, DIS_ADDR, DIS_FDS);

	| LDDF (addr, fdd) [name] => 
		stmts = instantiate(pc,	 name, DIS_ADDR, DIS_FDD);

	| load_asi (addr, asi, rd) [name] => 
		unused(asi);			// Note: this could be serious!
		stmts = instantiate(pc,	 name, DIS_RD, DIS_ADDR);

	| sto_greg(rd, addr) [name] => 
		// Note: RD is on the "right hand side" only for stores
		stmts = instantiate(pc,	 name, DIS_RDR, DIS_ADDR);

	| STF (fds, addr) [name] => 
		stmts = instantiate(pc,	 name, DIS_FDS, DIS_ADDR);

	| STDF (fdd, addr) [name] => 
		stmts = instantiate(pc,	 name, DIS_FDD, DIS_ADDR);

	| sto_asi (rd, addr, asi) [name] => 
		unused(asi);			// Note: this could be serious!
		stmts = instantiate(pc,	 name, DIS_RDR, DIS_ADDR);

	| LDFSR(addr) [name] => 
		stmts = instantiate(pc,	 name, DIS_ADDR);

	| LDCSR(addr) [name] => 
		stmts = instantiate(pc,	 name, DIS_ADDR);

	| STFSR(addr) [name] => 
		stmts = instantiate(pc,	 name, DIS_ADDR);

	| STCSR(addr) [name] => 
		stmts = instantiate(pc,	 name, DIS_ADDR);

	| STDFQ(addr) [name] => 
		stmts = instantiate(pc,	 name, DIS_ADDR);

	| STDCQ(addr) [name] => 
		stmts = instantiate(pc,	 name, DIS_ADDR);

	| RDY(rd) [name] => 
		stmts = instantiate(pc,	 name, DIS_RD);

	| RDPSR(rd) [name] => 
		stmts = instantiate(pc,	 name, DIS_RD);

	| RDWIM(rd) [name] => 
		stmts = instantiate(pc,	 name, DIS_RD);

	| RDTBR(rd) [name]	=> 
		stmts = instantiate(pc,	 name, DIS_RD);

	| WRY(rs1,roi) [name]	=> 
		stmts = instantiate(pc,	 name, DIS_RS1, DIS_ROI);

	| WRPSR(rs1, roi) [name] => 
		stmts = instantiate(pc,	 name, DIS_RS1, DIS_ROI);

	| WRWIM(rs1, roi) [name] => 
		stmts = instantiate(pc,	 name, DIS_RS1, DIS_ROI);

	| WRTBR(rs1, roi) [name] => 
		stmts = instantiate(pc,	 name, DIS_RS1, DIS_ROI);

	| alu (rs1, roi, rd) [name] => 
		stmts = instantiate(pc,	 name, DIS_RS1, DIS_ROI, DIS_RD);

	| float2s (fs2s, fds) [name] => 
		stmts = instantiate(pc,	 name, DIS_FS2S, DIS_FDS);

	| float3s (fs1s, fs2s, fds) [name] => 
		stmts = instantiate(pc,	 name, DIS_FS1S, DIS_FS2S, DIS_FDS);
 
	| float3d (fs1d, fs2d, fdd) [name] => 
		stmts = instantiate(pc,	 name, DIS_FS1D, DIS_FS2D, DIS_FDD);
 
	| float3q (fs1q, fs2q, fdq) [name] => 
		stmts = instantiate(pc,	 name, DIS_FS1Q, DIS_FS2Q, DIS_FDQ);
 
	| fcompares (fs1s, fs2s) [name] => 
		stmts = instantiate(pc,	 name, DIS_FS1S, DIS_FS2S);

	| fcompared (fs1d, fs2d) [name] => 
		stmts = instantiate(pc,	 name, DIS_FS1D, DIS_FS2D);

	| fcompareq (fs1q, fs2q) [name] => 
		stmts = instantiate(pc,	 name, DIS_FS1Q, DIS_FS2Q);

	| FTOs (fs2s, fds) [name] =>
		stmts = instantiate(pc, name, DIS_FS2S, DIS_FDS);

	// Note: itod and dtoi have different sized registers
	| FiTOd (fs2s, fdd) [name] =>
		stmts = instantiate(pc, name, DIS_FS2S, DIS_FDD);
	| FdTOi (fs2d, fds) [name] =>
		stmts = instantiate(pc, name, DIS_FS2D, DIS_FDS);

	| FiTOq (fs2s, fdq) [name] =>
		stmts = instantiate(pc, name, DIS_FS2S, DIS_FDQ);
	| FqTOi (fs2q, fds) [name] =>
		stmts = instantiate(pc, name, DIS_FS2Q, DIS_FDS);

	| FsTOd (fs2s, fdd) [name] =>
		stmts = instantiate(pc, name, DIS_FS2S, DIS_FDD);
	| FdTOs (fs2d, fds) [name] =>
		stmts = instantiate(pc, name, DIS_FS2D, DIS_FDS);

	| FsTOq (fs2s, fdq) [name] =>
		stmts = instantiate(pc, name, DIS_FS2S, DIS_FDQ);
	| FqTOs (fs2q, fds) [name] =>
		stmts = instantiate(pc, name, DIS_FS2Q, DIS_FDS);

	| FdTOq (fs2d, fdq) [name] =>
		stmts = instantiate(pc, name, DIS_FS2D, DIS_FDQ);
	| FqTOd (fs2q, fdd) [name] =>
		stmts = instantiate(pc, name, DIS_FS2Q, DIS_FDD);


	| FSQRTd (fs2d, fdd) [name] =>
		stmts = instantiate(pc, name, DIS_FS2D, DIS_FDD);

	| FSQRTq (fs2q, fdq) [name] =>
		stmts = instantiate(pc, name, DIS_FS2Q, DIS_FDQ);


	// In V9, the privileged RETT becomes user-mode RETURN
	// It has the semantics of "ret restore" without the add part of the restore
	| RETURN (addr) [name] => 
		stmts = instantiate(pc, name, DIS_ADDR);
		result.rtl = new RTL(pc, stmts);
		result.rtl->appendStmt(new ReturnStatement);
		result.type = DD;

	| trap (addr) [name] => 
		stmts = instantiate(pc,	 name, DIS_ADDR);

	| UNIMP (n) => 
		unused(n);
		stmts = NULL;
		result.valid = false;

	| inst = n => 
		// What does this mean?
		unused(n);
		result.valid = false;
		stmts = NULL;

	else
		stmts = NULL;
		result.valid = false;
		result.numBytes = 4;
	endmatch

	result.numBytes = nextPC - hostPC;
	if (result.valid && result.rtl == 0)	// Don't override higher level res
		result.rtl = new RTL(pc, stmts);

	return result;
}


/***********************************************************************
 * These are functions used to decode instruction operands into
 * expressions (Exp*s).
 **********************************************************************/

/*==============================================================================
 * FUNCTION:		SparcDecoder::dis_RegLhs
 * OVERVIEW:		Decode the register on the LHS
 * PARAMETERS:		r - register (0-31)
 * RETURNS:			the expression representing the register
 *============================================================================*/
Exp* SparcDecoder::dis_RegLhs(unsigned r)
{
	return Location::regOf(r);
}

/*==============================================================================
 * FUNCTION:		SparcDecoder::dis_RegRhs
 * OVERVIEW:		Decode the register on the RHS
 * NOTE:			Replaces r[0] with const 0
 * NOTE:			Not used by DIS_RD since don't want 0 on LHS
 * PARAMETERS:		r - register (0-31)
 * RETURNS:			the expression representing the register
 *============================================================================*/
Exp* SparcDecoder::dis_RegRhs(unsigned r)
{
	if (r == 0)
		return new Const(0);
	return Location::regOf(r);
}

/*==============================================================================
 * FUNCTION:		SparcDecoder::dis_RegImm
 * OVERVIEW:		Decode the register or immediate at the given address.
 * NOTE:			Used via macro DIS_ROI
 * PARAMETERS:		pc - an address in the instruction stream
 * RETURNS:			the register or immediate at the given address
 *============================================================================*/
Exp* SparcDecoder::dis_RegImm(ADDRESS pc)
{

	match pc to
	| imode(i) =>
		Exp* expr = new Const(i);
		return expr;
	| rmode(rs2) =>
		return dis_RegRhs(rs2);
	endmatch
}

/*==============================================================================
 * FUNCTION:		SparcDecoder::dis_Eaddr
 * OVERVIEW:		Converts a dynamic address to a Exp* expression.
 *					E.g. %o7 --> r[ 15 ]
 * PARAMETERS:		pc - the instruction stream address of the dynamic address
 *					ignore - redundant parameter on SPARC
 * RETURNS:			the Exp* representation of the given address
 *============================================================================*/
Exp* SparcDecoder::dis_Eaddr(ADDRESS pc, int ignore /* = 0 */)
{
	Exp* expr;

	match pc to
	| indirectA(rs1) =>
		expr = Location::regOf(rs1);
	| indexA(rs1, rs2) =>
		expr = new Binary(opPlus,
			Location::regOf(rs1),
			Location::regOf(rs2));
	| absoluteA(i) =>
		expr = new Const((int)i);
	| dispA(rs1,i) =>
		expr = new Binary(opPlus,
			Location::regOf(rs1),
			new Const((int)i));
	endmatch

	return expr;
}

/*==============================================================================
 * FUNCTION:	  isFuncPrologue()
 * OVERVIEW:	  Check to see if the instructions at the given offset match any callee prologue, i.e. does it look
 *					like this offset is a pointer to a function?
 * PARAMETERS:	  hostPC - pointer to the code in question (host address)
 * RETURNS:		  True if a match found
 *============================================================================*/
bool SparcDecoder::isFuncPrologue(ADDRESS hostPC)
{
#if 0		// Can't do this without patterns. It was a bit of a hack anyway
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
 * FUNCTION:	  isRestore()
 * OVERVIEW:	  Check to see if the instruction at the given offset is a restore instruction
 * PARAMETERS:	  hostPC - pointer to the code in question (host address)
 * RETURNS:		  True if a match found
 *============================================================================*/
bool SparcDecoder::isRestore(ADDRESS hostPC) {
		match hostPC to
		| RESTORE(a, b, c) =>
			unused(a);		// Suppress warning messages
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
 * FUNCTION:		getDword
 * OVERVIEW:		Returns the double starting at the given address.
 * PARAMETERS:		lc - address at which to decode the double
 * RETURNS:			the decoded double
 *============================================================================*/
DWord SparcDecoder::getDword(ADDRESS lc)
{
	Byte* p = (Byte*)lc;
	return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}

/*==============================================================================
 * FUNCTION:	   SparcDecoder::SparcDecoder
 * OVERVIEW:	   
 * PARAMETERS:	   None
 * RETURNS:		   N/A
 *============================================================================*/
SparcDecoder::SparcDecoder(Prog* prog) : NJMCDecoder(prog)
{
	std::string file = Boomerang::get()->getProgPath() + "frontend/machine/sparc/sparc.ssl";
	RTLDict.readSSLFile(file.c_str());
}

// For now...
int SparcDecoder::decodeAssemblyInstruction(ADDRESS, int)
{ return 0; }

