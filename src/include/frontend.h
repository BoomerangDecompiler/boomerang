#pragma once

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

/***************************************************************************/ /**
 * \file     frontend.h
 * \brief    This file contains the definition for the FrontEnd class, which implements the source indendent parts of
 *           the front end: decoding machine instructions into a control flow graph populated with low and high level
 *           RTLs.
 ******************************************************************************/
#include "include/types.h"
#include "sigenum.h" // For enums platform and cc
#include "boom_base/BinaryFile.h"
#include "TargetQueue.h"

#include <list>
#include <map>
#include <queue>
#include <fstream>
#include <QMap>
#include <memory>

class UserProc;
class Function;
class RTL;
class IInstructionTranslator;
class BasicBlock;
class Exp;
class TypedExp;
class Cfg;
class Prog;
struct DecodeResult;

class Signature;
class Instruction;
class CallStatement;
class SymTab;
using SharedExp      = std::shared_ptr<Exp>;
using SharedConstExp = std::shared_ptr<const Exp>;

// Control flow types
enum INSTTYPE
{
	I_UNCOND,   // unconditional branch
	I_COND,     // conditional branch
	I_N_COND,   // case branch
	I_CALL,     // procedure call
	I_RET,      // return
	I_COMPJUMP, // computed jump
	I_COMPCALL  // computed call
};

class IBinaryImage;
class FrontEnd
{
protected:
	IBinaryImage *Image;
	//      const int NOP_SIZE;            // Size of a no-op instruction (in bytes)
	//      const int NOP_INST;            // No-op pattern
	IInstructionTranslator *decoder; // The decoder
	/***************************************/
	// Loader interfaces
	/***************************************/
	LoaderInterface *ldrIface;
	QObject *pLoader; // The binary file
	/***************************************/

	BinaryFileFactory *pbff; // The binary file factory (for closing properly)
	Prog *Program;           // The Prog object
	// The queue of addresses still to be processed
	TargetQueue targetQueue;
	// Public map from function name (string) to signature.
	QMap<QString, std::shared_ptr<Signature> > LibrarySignatures;
	// Map from address to meaningful name
	std::map<ADDRESS, QString> refHints;
	// Map from address to previously decoded RTLs for decoded indirect control transfer instructions
	std::map<ADDRESS, RTL *> previouslyDecoded;

public:

	/*
	 * Constructor. Takes some parameters to save passing these around a lot
	 */
	FrontEnd(QObject *pLoader, Prog *prog, BinaryFileFactory *pbff);
	static FrontEnd *instantiate(QObject *pLoader, Prog *prog, BinaryFileFactory *pbff);
	static FrontEnd *Load(const QString& fname, Prog *prog); ///< Load a binary

	/// Add a symbol to the loader
	void AddSymbol(ADDRESS addr, const QString& nam);

	// Add a "hint" that an instruction at the given address references a named global
	void addRefHint(ADDRESS addr, const QString& nam) { refHints[addr] = nam; }
	virtual ~FrontEnd(); ///< Destructor. Virtual to mute a warning

	// returns a symbolic name for a register index
	QString getRegName(int idx) const;
	int getRegSize(int idx);

	// returns an enum identifer for this frontend's platform
	virtual Platform getFrontEndId() const = 0;
	bool isWin32() const; // Is this a win32 frontend?
	static bool noReturnCallDest(const QString& name);

	QObject *getBinaryFile() { return pLoader; }
	LoaderInterface *getLoaderIface() { return ldrIface; }

	// Function to fetch the smallest machine instruction
	// virtual    int            getInst(int addr);

	virtual DecodeResult& decodeInstruction(ADDRESS pc);

	virtual void extraProcessCall(CallStatement * /*call*/, std::list<RTL *> * /*BB_rtls*/) {}

	// Accessor function to get the decoder.
	IInstructionTranslator *getDecoder() { return decoder; }

	void readLibrarySignatures(const char *sPath, callconv cc); ///< Read library signatures from a file.
	void readLibraryCatalog(const QString& sPath);              ///< read from a catalog
	void readLibraryCatalog();                                  ///< read from default catalog

	// lookup a library signature by name
	std::shared_ptr<Signature> getLibSignature(const QString& name);

	// return a signature that matches the architecture best
	std::shared_ptr<Signature> getDefaultSignature(const QString& name);

	virtual std::vector<SharedExp>& getDefaultParams() = 0;

	virtual std::vector<SharedExp>& getDefaultReturns() = 0;

	/*
	 * Decode all undecoded procedures and return a new program containing them.
	 */
	void decode(Prog *Program, bool decodeMain = true, const char *pname = nullptr);

	// Decode all procs starting at a given address in a given program.
	void decode(Prog *Program, ADDRESS a);

	// Decode one proc starting at a given address in a given program.
	void decodeOnly(Prog *Program, ADDRESS a);

	// Decode a fragment of a procedure, e.g. for each destination of a switch statement
	void decodeFragment(UserProc *proc, ADDRESS a);

	virtual bool processProc(ADDRESS uAddr, UserProc *pProc, QTextStream& os, bool frag = false, bool spec = false);

	/**
	 * Given the dest of a call, determine if this is a machine specific helper function with special semantics.
	 * If so, return true and set the semantics in lrtl.  addr is the native address of the call instruction
	 */
	virtual bool helperFunc(ADDRESS /*dest*/, ADDRESS /*addr*/, std::list<RTL *> * /*lrtl*/) { return false; }

	/**
	 * Locate the starting address of "main", returning a native address
	 */
	virtual ADDRESS getMainEntryPoint(bool& gotMain) = 0;

	/**
	 * Returns a list of all available entrypoints.
	 */
	std::vector<ADDRESS> getEntryPoints();

//    /**
//     * Get an instance of a class derived from FrontEnd, returning a pointer to the object of
//     * that class. Do this by guessing the machine for the binary file whose name is sName, loading the
//     * appropriate library using dlopen/dlsym, running the "construct" function in that library, and returning
//     * the result.
//     */
//    static FrontEnd *getInstanceFor(const char *sName, void *&dlHandle, QObject *pLoader, IInstructionTranslator *&decoder);
	static void closeInstance(void *dlHandle); ///< Close the library opened by getInstanceFor
	Prog *getProg();                           /// Get a Prog object (for testing and not decoding)

	/**
	 * Create a Return or a Oneway BB if a return statement already exists
	 * \param pProc pointer to enclosing UserProc
	 * \param BB_rtls list of RTLs for the current BB
	 * \param pRtl pointer to the current RTL with the semantics for the return statement (including a
	 *             ReturnStatement as the last statement)
	 */
	BasicBlock *createReturnBlock(UserProc *pProc, std::list<RTL *> *BB_rtls, RTL *pRtl);

	/**
	 * Add a synthetic return instruction and basic block (or a branch to the existing return instruction).
	 * \param pCallBB a pointer to the call BB that will be followed by the return or jump
	 * \param pProc pointer to the enclosing UserProc
	 * \param pRtl pointer to the current RTL with the call instruction
	 */
	void appendSyntheticReturn(BasicBlock *pCallBB, UserProc *pProc, RTL *pRtl);

	/**
	 * Add an RTL to the map from native address to previously-decoded-RTLs. Used to restore case statements and
	 * decoded indirect call statements in a new decode following analysis of such instructions. The CFG is
	 * incomplete in these cases, and needs to be restarted from scratch
	 */
	void addDecodedRtl(ADDRESS a, RTL *rtl) { previouslyDecoded[a] = rtl; }
	void preprocessProcGoto(std::list<Instruction *>::iterator ss, ADDRESS dest, const std::list<Instruction *>& sl,
							RTL *pRtl);
	void checkEntryPoint(std::vector<ADDRESS>& entrypoints, ADDRESS addr, const char *type);

private:
	bool refersToImportedFunction(const SharedExp& pDest);

	SymTab *BinarySymbols;
}; // class FrontEnd

/***************************************************************************/ /**
 * These functions are the machine specific parts of the front end. They consist
 * of those that actually drive the decoding and analysis of the procedures of
 * the program being translated.
 * These functions are implemented in the files front<XXX> where XXX is a
 * platform name such as sparc or pentium.
 ******************************************************************************/
