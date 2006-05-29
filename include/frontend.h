/*
 * Copyright (C) 1998-2005, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc  
 * Copyright (C) 2002, Trent Waddington
 *
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:		frontend.h
 * OVERVIEW:	This file contains the definition for the FrontEnd class, which implements the source indendent parts of
 *				the front end: decoding machine instructions into a control flow graph populated with low and high level
 *			    RTLs.
 *============================================================================*/

/* $Revision$	// 1.29.2.2
 *
 * 17 Apr 02 - Mike: Mods to adapt UQBT code to boomerang
 * 28 Jun 05 - Mike: Added a map of previously decoded indirect jumps and calls needed when restarting the cfg
 */


#ifndef __FRONTEND_H__
#define __FRONTEND_H__

#include <list>
#include <map>
#include <queue>
#include <fstream>
#include "types.h"
#include "sigenum.h"   // For enums platform and cc
#include "BinaryFile.h"

class UserProc;
class Proc;
class RTL;
class NJMCDecoder;
class BasicBlock;
typedef BasicBlock* PBB;
class Exp;
class TypedExp;
class Cfg;
class Prog;
struct DecodeResult;
class Signature;
class Statement;
class CallStatement;

// Control flow types
enum INSTTYPE {
	I_UNCOND,				 // unconditional branch
	I_COND,					 // conditional branch
	I_N_COND,				 // case branch
	I_CALL,					 // procedure call
	I_RET,					 // return
	I_COMPJUMP,				 // computed jump
	I_COMPCALL				 // computed call
};

// Put the target queue logic into this small class
class TargetQueue {
		std::queue<ADDRESS> targets;

public:

/*
 * FUNCTION:	visit
 * OVERVIEW:	Visit a destination as a label, i.e. check whether we need to queue it as a new BB to create later.
 *				Note: at present, it is important to visit an address BEFORE an out edge is added to that address.
 *				This is because adding an out edge enters the address into the Cfg's BB map, and it looks like the
 *				BB has already been visited, and it gets overlooked. It would be better to have a scheme whereby
 *				the order of calling these functions (i.e. visit() and AddOutEdge()) did not matter.
 * PARAMETERS:	pCfg - the enclosing CFG
 *				uNewAddr - the address to be checked
 *				pNewBB - set to the lower part of the BB if the address already exists as a non explicit label
 *				(i.e. the BB has to be split)
 * RETURNS:		<nothing>
 */
	void visit(Cfg* pCfg, ADDRESS uNewAddr, PBB& pNewBB);
/*
 * Provide an initial address (can call several times if there are several entry points)
 */
		void		initial(ADDRESS uAddr);


/*
 * FUNCTION:	  nextAddress
 * OVERVIEW:	  Return the next target from the queue of non-processed targets.
 * PARAMETERS:	  cfg - the enclosing CFG
 * RETURNS:		  The next address to process, or 0 if none (queue is empty)
 */
		ADDRESS		nextAddress(Cfg* cfg);

/*
 * Print (for debugging)
 */
		void		dump();

};	// class TargetQueue


typedef bool (*PHELPER)(ADDRESS dest, ADDRESS addr, std::list<RTL*>* lrtl); 

class FrontEnd {
protected:
//	  const int NOP_SIZE;			// Size of a no-op instruction (in bytes)
//	  const int NOP_INST;			// No-op pattern
		NJMCDecoder	*decoder;		// The decoder
		BinaryFile	*pBF;			// The binary file
		BinaryFileFactory* pbff;	// The binary file factory (for closing properly)
		Prog*		prog;			// The Prog object
		// The queue of addresses still to be processed
		TargetQueue	targetQueue;
		// Public map from function name (string) to signature.
		std::map<std::string, Signature*> librarySignatures;
		// Map from address to meaningful name
		std::map<ADDRESS, std::string> refHints;
		// Map from address to previously decoded RTLs for decoded indirect control transfer instructions
		std::map<ADDRESS, RTL*> previouslyDecoded;
public:
		/*
		 * Constructor. Takes some parameters to save passing these around a lot
		 */
					FrontEnd(BinaryFile *pBF, Prog* prog, BinaryFileFactory* pbff);
		// Create from a binary file
static	FrontEnd*	instantiate(BinaryFile *pBF, Prog* prog, BinaryFileFactory* pbff);
		// Load a binary
static	FrontEnd*	Load(const char *fname, Prog* prog);

		// Add a symbol to the loader
		void		AddSymbol(ADDRESS addr, const char *nam) { pBF->AddSymbol(addr, nam); }

		// Add a "hint" that an instruction at the given address references a named global
		void		addRefHint(ADDRESS addr, const char *nam) { refHints[addr] = nam; }

		/**
		 * Destructor. Virtual to mute a warning
		 */
virtual				~FrontEnd();

		// returns a symbolic name for a register index
		const char	*getRegName(int idx);

		// returns an enum identifer for this frontend's platform
virtual platform	getFrontEndId() = 0;

		// returns a frontend given a string (unused?)
static FrontEnd		*createById(std::string &str, BinaryFile *pBFi, Prog* prog);

		bool		isWin32();					// Is this a win32 frontend?

		BinaryFile 	*getBinaryFile() { return pBF; }

		/*
		 * Function to fetch the smallest machine instruction
		 */
virtual	int			getInst(int addr);

virtual DecodeResult& decodeInstruction(ADDRESS pc);

virtual void extraProcessCall(CallStatement *call, std::list<RTL*> *BB_rtls) { }

		/*
		 * Accessor function to get the decoder.
		 */
		NJMCDecoder *getDecoder() { return decoder; }

		/*
		 * Read library signatures from a file.
		 */
		void		readLibrarySignatures(const char *sPath, callconv cc);
		// read from a catalog
		void		readLibraryCatalog(const char *sPath);
		// read from default catalog
		void		readLibraryCatalog();

		// lookup a library signature by name
		Signature	*getLibSignature(const char *name);

		// return a signature that matches the architecture best
		Signature	*getDefaultSignature(const char *name);

virtual std::vector<Exp*> &getDefaultParams() = 0;
virtual std::vector<Exp*> &getDefaultReturns() = 0;

		/*
		 * Decode all undecoded procedures and return a new program containing them.
		 */
		void		decode(Prog* prog, bool decodeMain = true, const char *pname = NULL);

		/* Decode all procs starting at a given address in a given program. */
		void		decode(Prog *prog, ADDRESS a);

		/* Decode one proc starting at a given address in a given program. */
		void		decodeOnly(Prog *prog, ADDRESS a);

		/* Decode a fragment of a procedure, e.g. for each destination of a switch statement */
		void		decodeFragment(UserProc* proc, ADDRESS a);

		/*
		 * processProc. This is the main function for decoding a procedure. It is usually overridden in the derived
		 * class to do source machine specific things.  If frag is set, we are decoding just a fragment of the proc
		 * (e.g. each arm of a switch statement is decoded). If spec is set, this is a speculative decode.
		 * Returns true on a good decode
		 */
virtual bool		processProc(ADDRESS uAddr, UserProc* pProc, std::ofstream &os, bool frag = false,
						bool spec = false);

		/*
		 * Given the dest of a call, determine if this is a machine specific helper function with special semantics.
		 * If so, return true and set the semantics in lrtl.  addr is the native address of the call instruction
		 */
virtual bool		helperFunc(ADDRESS dest, ADDRESS addr, std::list<RTL*>* lrtl) {return false; }

		/*
		 * Locate the starting address of "main", returning a native address
		 */
virtual	ADDRESS		getMainEntryPoint( bool &gotMain ) = 0;

		/*
		 * Returns a list of all available entrypoints.
		 */
std::vector<ADDRESS> getEntryPoints();

		/*
		 * getInstanceFor. Get an instance of a class derived from FrontEnd, returning a pointer to the object of
		 * that class. Do this by guessing the machine for the binary file whose name is sName, loading the
		 * appropriate library using dlopen/dlsym, running the "construct" function in that library, and returning
		 * the result.
		 */
static	FrontEnd*	getInstanceFor( const char* sName, void*& dlHandle, BinaryFile *pBF, NJMCDecoder*& decoder);

		/*
		 * Close the library opened by getInstanceFor
		 */
static	void		closeInstance(void* dlHandle);

		/*
		 * Get a Prog object (for testing and not decoding)
		 */
		Prog*		getProg();

		/*
		 * Create a Return or a Oneway BB if a return statement already exists
		 * PARAMETERS:	pProc: pointer to enclosing UserProc
		 *				BB_rtls: list of RTLs for the current BB
		 *				pRtl: pointer to the current RTL with the semantics for the return statement (including a
		 *					ReturnStatement as the last statement)
		 */
		PBB			createReturnBlock(UserProc* pProc, std::list<RTL*>* BB_rtls, RTL* pRtl);

		/*
		 * Add a synthetic return instruction and basic block (or a branch to the existing return instruction).
		 * PARAMETERS:	pCallBB: a pointer to the call BB that will be followed by the return or jump
		 *				pProc: pointer to the enclosing UserProc
		 *				pRtl: pointer to the current RTL with the call instruction
		 */
		void		appendSyntheticReturn(PBB pCallBB, UserProc* pProc, RTL* pRtl);

		/*
		 * Add an RTL to the map from native address to previously-decoded-RTLs. Used to restore case statements and
		 * decoded indirect call statements in a new decode following analysis of such instructions. The CFG is
		 * incomplete in these cases, and needs to be restarted from scratch
		 */
		void		addDecodedRtl(ADDRESS a, RTL* rtl) {
						previouslyDecoded[a] = rtl; }

};	// class FrontEnd


/*==============================================================================
 * These functions are the machine specific parts of the front end. They consist
 * of those that actually drive the decoding and analysis of the procedures of
 * the program being translated.
 * These functions are implemented in the files front<XXX> where XXX is a
 * platform name such as sparc or pentium.
 *============================================================================*/

		/*
		 * Intialise the procedure decoder and analyser.
		 */
		void		initFront();

		/*
		 * Decode one RTL
		 */
		RTL*		decodeRtl(ADDRESS address, int delta, NJMCDecoder* decoder);

		/*
		 * This decodes a given procedure. It performs the analysis to recover switch statements, call
		 * parameters and return types etc.
		 * If keep is false, discard the decoded procedure (only need this to find code other than main that is
		 * reachable from _start, for coverage and speculative decoding)
		 * If spec is true, then we are speculatively decoding (i.e. if there is an illegal instruction, we just bail
		 * out)
		 */
		bool		decodeProc(ADDRESS uAddr, FrontEnd& fe, bool keep = true, bool spec = false);


#endif		// #ifndef __FRONTEND_H__
