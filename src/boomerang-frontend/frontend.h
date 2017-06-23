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

#include "boomerang/include/types.h"
#include "boomerang/include/sigenum.h" // For enums platform and cc
#include "boomerang/core/BinaryFileFactory.h"
#include "boomerang/include/TargetQueue.h"

#include <list>
#include <map>
#include <queue>
#include <fstream>
#include <QMap>
#include <memory>

class UserProc;
class Function;
class RTL;
class IDecoder;
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
class IBinaryImage;

using SharedExp      = std::shared_ptr<Exp>;
using SharedConstExp = std::shared_ptr<const Exp>;

// Control flow types
enum INSTTYPE
{
	I_UNCOND,   ///< unconditional branch
	I_COND,     ///< conditional branch
	I_N_COND,   ///< case branch
	I_CALL,     ///< procedure call
	I_RET,      ///< return
	I_COMPJUMP, ///< computed jump
	I_COMPCALL  ///< computed call
};


class IFrontEnd
{
public:
	/***************************************************************************/ /**
	* \brief      Construct the FrontEnd object
	* \param loader pointer to the Loader
	* \param prog program being decoded
	* \param bff  pointer to a BinaryFileFactory object (so the library can be unloaded)
	******************************************************************************/
	IFrontEnd(IFileLoader *loader, Prog *prog, BinaryFileFactory *bff);
	
	virtual ~IFrontEnd();


	/***************************************************************************/ /**
	* \brief Create from a binary file
	* Static function to instantiate an appropriate concrete front end
	* \param loader pointer to the loader object
	* \param prog program being decoded
	* \param bff pointer to a BinaryFileFactory object (so the library can be unloaded)
	******************************************************************************/
	static IFrontEnd *instantiate(IFileLoader *loader, Prog *prog, BinaryFileFactory *bff);
	
	/***************************************************************************/ /**
	* \brief Create FrontEnd instance given \a fname and \a prog
	* 
	* \param fname string with full path to decoded file
	* \param prog program being decoded
	* \returns Binary-specific frontend.
	******************************************************************************/
	static IFrontEnd *create(const QString& fname, Prog *prog);

//    /**
//     * Get an instance of a class derived from FrontEnd, returning a pointer to the object of
//     * that class. Do this by guessing the machine for the binary file whose name is sName, loading the
//     * appropriate library using dlopen/dlsym, running the "construct" function in that library, and returning
//     * the result.
//     */
//    static FrontEnd *getInstanceFor(const char *sName, void *&dlHandle, QObject *pLoader, IInstructionTranslator *&decoder);
	
	/// Is this a win32 frontend?
	bool isWin32() const;
	
	static bool isNoReturnCallDest(const QString& name);

	/// \returns an enum identifer for this frontend's platform
	virtual Platform getType() const = 0;

	IFileLoader *getLoader() const { return m_fileLoader; }

	/// Accessor function to get the decoder.
	IDecoder *getDecoder() { return m_decoder; }

	/// returns a symbolic name for a register index
	QString getRegName(int idx) const;
	int getRegSize(int idx);

	
	/// lookup a library signature by name
	/// get a library signature by name
	std::shared_ptr<Signature> getLibSignature(const QString& name);

	/// return a signature that matches the architecture best
	std::shared_ptr<Signature> getDefaultSignature(const QString& name);

	virtual std::vector<SharedExp>& getDefaultParams() = 0;

	virtual std::vector<SharedExp>& getDefaultReturns() = 0;
	
	
	/// Add a symbol to the loader
	void addSymbol(ADDRESS addr, const QString& nam);

	/// Add a "hint" that an instruction at the given address references a named global
	void addRefHint(ADDRESS addr, const QString& nam) { m_refHints[addr] = nam; }

	// Function to fetch the smallest machine instruction
	// virtual    int            getInst(int addr);

	virtual DecodeResult& decodeInstruction(ADDRESS pc);

	virtual void extraProcessCall(CallStatement * /*call*/, std::list<RTL *> * /*BB_rtls*/) {}


	/***************************************************************************/ /**
	* \brief       Read the library signatures from a file
	* \param       sPath The file to read from
	* \param       cc the calling convention assumed
	******************************************************************************/
	void readLibrarySignatures(const char *sPath, CallConv cc); ///< Read library signatures from a file.
	void readLibraryCatalog(const QString& sPath);              ///< read from a catalog
	void readLibraryCatalog();                                  ///< read from default catalog

	/**
	 * Decode all undecoded procedures and return a new program containing them.
	 * Somehow, a == NO_ADDRESS has come to mean decode anything not already decoded
	 */
	void decode(Prog *Program, bool decodeMain = true, const char *pname = nullptr);

	/// Decode all procs starting at a given address in a given program.
	void decode(Prog *Program, ADDRESS a);

	/// Decode one proc starting at a given address in a given program.
	/// \a a should be the address of an UserProc
	void decodeOnly(Prog *Program, ADDRESS a);

	/// Decode a fragment of a procedure, e.g. for each destination of a switch statement
	void decodeFragment(UserProc *proc, ADDRESS a);

	
	/***************************************************************************/ /**
	* \brief      Process a procedure, given a native (source machine) address.
	*
	* This is the main function for decoding a procedure. It is usually overridden in the derived
	* class to do source machine specific things.  If frag is set, we are decoding just a fragment of the proc
	* (e.g. each arm of a switch statement is decoded). If spec is set, this is a speculative decode.
	* 
	* \param addr - the address at which the procedure starts
	* \param proc - the procedure object
	* \param os - the output stream for .rtl output
	* \param frag - if true, this is just a fragment of a procedure
	* \param spec - if true, this is a speculative decode
	* 
	* \note This is a sort of generic front end. For many processors, this will be overridden
	*  in the FrontEnd derived class, sometimes calling this function to do most of the work.
	* 
	* \returns          true for a good decode (no illegal instructions)
	******************************************************************************/
	virtual bool processProc(ADDRESS addr, UserProc *proc, QTextStream& os, bool frag = false, bool spec = false);

	/**
	 * Given the dest of a call, determine if this is a machine specific helper function with special semantics.
	 * If so, return true and set the semantics in lrtl.
	 * 
	 * @param addr the native address of the call instruction
	 */
	virtual bool isHelperFunc(ADDRESS /*dest*/, ADDRESS /*addr*/, std::list<RTL *> * /*lrtl*/) { return false; }

	/**
	 * Locate the starting address of "main", returning a native address
	 */
	virtual ADDRESS getMainEntryPoint(bool& gotMain) = 0;

	/**
	 * Returns a list of all available entrypoints.
	 */
	std::vector<ADDRESS> getEntryPoints();

	static void closeInstance(void *dlHandle); ///< Close the library opened by getInstanceFor
	
	/***************************************************************************/ /**
	* \brief    Get a Prog object (mainly for testing and not decoding)
	* \returns  Pointer to a Prog object (with pFE and pBF filled in)
	******************************************************************************/
	Prog *getProg();


	/***************************************************************************/ /**
	* \brief    Create a Return or a Oneway BB if a return statement already exists
	* \param    pProc pointer to enclosing UserProc
	* \param    BB_rtls list of RTLs for the current BB (not including pRtl)
	* \param    pRtl pointer to the current RTL with the semantics for the return statement (including a
	*           ReturnStatement as the last statement)
	* 
	* \returns  Pointer to the newly created BB
	******************************************************************************/
	BasicBlock *createReturnBlock(UserProc *pProc, std::list<RTL *> *BB_rtls, RTL *pRtl);

	/**
	 * Add a synthetic return instruction and basic block (or a branch to the existing return instruction).
	 * 
	 * \note the call BB should be created with one out edge (the return or branch BB)
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
	void addDecodedRtl(ADDRESS a, RTL *rtl) { m_previouslyDecoded[a] = rtl; }
	void preprocessProcGoto(std::list<Instruction *>::iterator ss, ADDRESS dest,
							const std::list<Instruction *>& sl, RTL *pRtl);
	void checkEntryPoint(std::vector<ADDRESS>& entrypoints, ADDRESS addr, const char *type);

private:
	bool refersToImportedFunction(const SharedExp& pDest);

protected:
	IBinaryImage *m_image;
	
//	const int NOP_SIZE;            ///< Size of a no-op instruction (in bytes)
//	const int NOP_INST;            ///< No-op pattern
	
	IDecoder *m_decoder; ///< The decoder
	
	IFileLoader *m_fileLoader;
	
	BinaryFileFactory *m_bff; ///< The binary file factory (for closing properly)
	Prog *m_program;          ///< The Prog object
	
	/// The queue of addresses still to be processed
	TargetQueue m_targetQueue;
	
	/// Public map from function name (string) to signature.
	QMap<QString, std::shared_ptr<Signature> > m_librarySignatures;
	
	/// Map from address to meaningful name
	std::map<ADDRESS, QString> m_refHints;
	
	/// Map from address to previously decoded RTLs for decoded indirect control transfer instructions
	std::map<ADDRESS, RTL *> m_previouslyDecoded;

private:
	SymTab *m_binarySymbols;
};
