/*
 * Copyright (C) 1999-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   frontend.cpp
 * OVERVIEW:   This file contains common code for all front ends. The majority
 *				of frontend logic remains in the source dependent files such as
 *				frontsparc.cpp
 *============================================================================*/

/*
 * $Revision$
 * 08 Apr 02 - Mike: Mods to adapt UQBT code to boomerang
 * 16 May 02 - Mike: Moved getMainEntry point here from prog
 * 09 Jul 02 - Mike: Fixed machine check for elf files (was checking endianness rather than machine type)
 * 22 Nov 02 - Mike: Quelched warnings
 * 16 Apr 03 - Mike: trace (-t) to cerr not cout now
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include <queue>
#include <stdarg.h>			// For varargs
#include <sstream>
#ifndef WIN32
#include <dlfcn.h>			// dlopen, dlsym
#endif

#include "types.h"
#include "exp.h"
#include "cfg.h"
#include "proc.h"
#include "register.h"
#include "rtl.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "decoder.h"
#include "sparcfrontend.h"
#include "pentiumfrontend.h"
#include "prog.h"
#include "signature.h"
#include "boomerang.h"
#include "ansi-c-parser.h"

/*==============================================================================
 * FUNCTION:	  FrontEnd::FrontEnd
 * OVERVIEW:	  Construct the FrontEnd object
 * PARAMETERS:	  prog: program being decoded
 * RETURNS:		  <N/a>
 *============================================================================*/
FrontEnd::FrontEnd(BinaryFile *pBF)
 : pBF(pBF)
{}

FrontEnd* FrontEnd::instantiate(BinaryFile *pBF) {
	switch(pBF->GetMachine()) {
	case MACHINE_PENTIUM:
		return new PentiumFrontEnd(pBF);
	case MACHINE_SPARC:
		return new SparcFrontEnd(pBF);
	default:
		LOG << "Machine architecture not supported\n";
	}
	return NULL;
}

FrontEnd* FrontEnd::Load(const char *fname) {
	BinaryFile *pBF = BinaryFile::Load(fname);
	if (pBF == NULL) return NULL;
	return instantiate(pBF);
}

// destructor
FrontEnd::~FrontEnd() {
}

const char *FrontEnd::getRegName(int idx) { 
	std::map<std::string, int, std::less<std::string> >::iterator it;
	for (it = decoder->getRTLDict().RegMap.begin();	 
		 it != decoder->getRTLDict().RegMap.end(); it++)
		if ((*it).second == idx) 
			return (*it).first.c_str();
	return NULL;
}

bool FrontEnd::isWin32() {
	return pBF->GetFormat() == LOADFMT_PE;
}

FrontEnd *FrontEnd::createById(std::string &str, BinaryFile *pBF) {
	if (str == "pentium")
		return new PentiumFrontEnd(pBF);
	if (str == "sparc")
		return new SparcFrontEnd(pBF);
	return NULL;
}

void FrontEnd::readLibraryCatalog(const char *sPath) {
	std::ifstream inf(sPath);
	if (!inf.good()) {
		LOG << "can't open `" << sPath << "'\n";
		exit(1);
	}

	while (!inf.eof()) {
		std::string sFile;
		inf >> sFile;
		size_t j = sFile.find('#');
		if (j != (size_t)-1)
			sFile = sFile.substr(0, j);
		if (sFile.size() > 0 && sFile[sFile.size()-1] == '\n')
			sFile = sFile.substr(0, sFile.size()-1);
		if (sFile == "") continue;
		std::string sPath = Boomerang::get()->getProgPath() + "signatures/" + sFile;
		callconv cc = CONV_C;			// Most APIs are C calling convention
		if (sFile == "windows.h")	cc = CONV_PASCAL;		// One exception
		if (sFile == "mfc.h")		cc = CONV_THISCALL;		// Another exception
		readLibrarySignatures(sPath.c_str(), cc);
	}
	inf.close();
}

void FrontEnd::readLibraryCatalog() {
	librarySignatures.clear();
	std::string sList = Boomerang::get()->getProgPath() + "signatures/common.hs";
	readLibraryCatalog(sList.c_str());
	sList = Boomerang::get()->getProgPath() + "signatures/" + 
		Signature::platformName(getFrontEndId()) + ".hs";
	readLibraryCatalog(sList.c_str());
	if (isWin32()) {
		sList = Boomerang::get()->getProgPath() + "signatures/win32.hs";
		readLibraryCatalog(sList.c_str());
	}
}

Prog *FrontEnd::decode(bool decodeMain, const char *pname) 
{
	Prog *prog = new Prog(pBF, this);
	if (pname)
		prog->setName(pname);
	readLibraryCatalog();

	if (!decodeMain)
		return prog;
	
	Boomerang::get()->alert_start_decode(pBF->getLimitTextLow(), pBF->getLimitTextHigh() - pBF->getLimitTextLow());

	bool gotMain;
	ADDRESS a = getMainEntryPoint(gotMain);
	if (VERBOSE)
		LOG << "start: " << a << " gotmain: " << (gotMain ? "true" : "false") << "\n";
	if (a == NO_ADDRESS) return false;

	decode(prog, a);

	if (gotMain) {
		static const char *mainName[] = { "main", "WinMain", "DriverEntry" };
		const char *name = pBF->SymbolByAddress(a);
		if (name == NULL)
			name = mainName[0];
		for (size_t i = 0; i < sizeof(mainName)/sizeof(char*); i++) {
			if (!strcmp(name, mainName[i])) {
				Proc *proc = prog->findProc(a);
				assert(proc);
				FuncType *fty = dynamic_cast<FuncType*>(Type::getNamedType(name));
				if (fty == NULL)
					LOG << "unable to find signature for known entrypoint " << name << "\n";
				else {
					proc->setSignature(fty->getSignature()->clone());
					proc->getSignature()->setName(name);
					proc->getSignature()->setFullSig(true);		// Don't add or remove parameters
				}
				break;
			}
		}
	}
	return prog;
}

void FrontEnd::decode(Prog *prog, ADDRESS a) {
	if (a != NO_ADDRESS) {
		prog->setNewProc(a);
		if (VERBOSE)
			LOG << "starting decode at address " << a << "\n";
	}

	bool change = true;
	while (change) {
		change = false;
		PROGMAP::const_iterator it;
		for (Proc *pProc = prog->getFirstProc(it); pProc != NULL; pProc = prog->getNextProc(it)) {
			if (pProc->isLib()) continue;
			UserProc *p = (UserProc*)pProc;
			if (p->isDecoded()) continue;

			// undecoded userproc.. decode it			
			change = true;
			std::ofstream os;
			int res = processProc(p->getNativeAddress(), p, os);
			if (res == 1)
				p->setDecoded();
			else
				break;
			// Break out of the loops if not decoding children
			if (Boomerang::get()->noDecodeChildren)
				break;
		}
		if (Boomerang::get()->noDecodeChildren)
			break;
	}
	prog->wellForm();
}

// a should be the address of a UserProc
void FrontEnd::decodeOnly(Prog *prog, ADDRESS a) {
	UserProc* p = (UserProc*)prog->setNewProc(a);
	assert(!p->isLib());
	std::ofstream os;
	if (processProc(p->getNativeAddress(), p, os))
		p->setDecoded();
	prog->wellForm();
}


void FrontEnd::decodeFragment(UserProc* proc, ADDRESS a) {
	std::ofstream os;
	processProc(a, proc, os, true);
}

DecodeResult& FrontEnd::decodeInstruction(ADDRESS pc) {
	return decoder->decodeInstruction(pc, pBF->getTextDelta());
}

/*==============================================================================
 * FUNCTION:	   FrontEnd::readLibrarySignatures
 * OVERVIEW:	   Read the library signatures from a file
 * PARAMETERS:	   sPath: The file to read from
 *				   cc: the calling convention assumed
 * RETURNS:		   <nothing>
 *============================================================================*/
void FrontEnd::readLibrarySignatures(const char *sPath, callconv cc) {
	std::ifstream ifs;

	ifs.open(sPath);

	if (!ifs.good()) {
		LOG << "can't open `" << sPath << "'\n";
		exit(1);
	}

	AnsiCParser *p = new AnsiCParser(ifs, false);
	
	platform plat = getFrontEndId();
	p->yyparse(plat, cc);

	for (std::list<Signature*>::iterator it = p->signatures.begin(); it != p->signatures.end(); it++) {
#if 0
		std::cerr << "readLibrarySignatures from " << sPath << ": " << (*it)->getName() << "\n";
#endif
		librarySignatures[(*it)->getName()] = *it;
	}

	delete p;
	ifs.close();
}

Signature *FrontEnd::getDefaultSignature(const char *name)
{
	Signature *signature = NULL;
	// Get a default library signature
	if (isWin32())
		signature = Signature::instantiate(PLAT_PENTIUM, CONV_PASCAL, name);
	else {
		signature = Signature::instantiate(getFrontEndId(), CONV_C, name);
	} 
	return signature;
}

// get a library signature by name
Signature *FrontEnd::getLibSignature(const char *name) {
	Signature *signature;
	// Look up the name in the librarySignatures map
	std::map<std::string, Signature*>::iterator it;
	it = librarySignatures.find(name);
	if (it == librarySignatures.end()) {
		LOG << "unknown library function " << name << "\n";
		signature = getDefaultSignature(name);
	}
	else {
		signature = (*it).second->clone();
		signature->setUnknown(false);
	}
	return signature;
}

#if 0		// Note: moved to Prog::setNewProc
/*==============================================================================
 * FUNCTION:	FrontEnd::newProc
 * OVERVIEW:	Call this function when a procedure is discovered (usually by
 *				  decoding a call instruction). That way, it is given a name
 *				  that can be displayed in the dot file, etc. If we assign it
 *				  a number now, then it will retain this number always
 * PARAMETERS:	prog  - program to add the new procedure to
 *				uAddr - Native address of the procedure entry point
 * RETURNS:		Pointer to the Proc object, or 0 if this is a deleted (not to
 *				  be decoded) address
 *============================================================================*/
Proc* FrontEnd::newProc(Prog *prog, ADDRESS uAddr) {
	// this test fails when decoding sparc, why?  Please investigate - trent
	//assert(uAddr >= limitTextLow && uAddr < limitTextHigh);
	// Check if we already have this proc
	Proc* pProc = prog->findProc(uAddr);
	if (pProc == (Proc*)-1)			// Already decoded and deleted?
		return 0;					// Yes, exit with 0
	if (pProc)
		// Yes, we are done
		return pProc;
	char* pName = pBF->SymbolByAddress(uAddr);
	bool bLib = pBF->IsDynamicLinkedProc(uAddr);
	if (pName == 0) {
		// No name. Give it a numbered name
		std::ostringstream ost;
		ost << "proc" << m_iNumberedProc++;
		pName = strdup(ost.str().c_str());
	}
	pProc = prog->newProc(pName, uAddr, bLib);
	return pProc;
}
#endif


/*==============================================================================
 * FUNCTION:	  FrontEnd::processProc
 * OVERVIEW:	  Process a procedure, given a native (source machine) address.
 * PARAMETERS:	  address - the address at which the procedure starts
 *				  pProc - the procedure object
 *				  frag - if true, this is just a fragment of a procedure
 *				  spec - if true, this is a speculative decode
 *				  os - the output stream for .rtl output
 * NOTE:		  This is a sort of generic front end. For many processors, this will be overridden
 *					in the FrontEnd derived class, sometimes calling this function to do most of the work
 * RETURNS:		  true for a good decode (no illegal instructions)
 *============================================================================*/
bool FrontEnd::processProc(ADDRESS uAddr, UserProc* pProc, std::ofstream &os, bool frag /* = false */,
		bool spec /* = false */) {
	PBB pBB;					// Pointer to the current basic block

	// if (!frag && !pProc->getSignature()->isPromoted()) {	// }
	if (!frag) {
		if (VERBOSE)
			LOG << "adding default params and returns for " << pProc->getName() << "\n";
		std::vector<Exp*> &params = getDefaultParams();
		std::vector<Exp*>::iterator it;
		for (it = params.begin(); it != params.end(); it++)
			pProc->getSignature()->addImplicitParameter((*it)->clone());
		std::vector<Exp*> &returns = getDefaultReturns();
		for (it = returns.begin(); it != returns.end(); it++)
			pProc->getSignature()->addReturn((*it)->clone());
	}
	
	// We have a set of CallStatement pointers. These may be disregarded if this is a speculative decode
	// that fails (i.e. an illegal instruction is found). If not, this set will be used to add to the set of calls
	// to be analysed in the cfg, and also to call newProc()
	std::set<CallStatement*> callSet;

	// Indicates whether or not the next instruction to be decoded is the lexical successor of the current one.
	// Will be true for all NCTs and for CTIs with a fall through branch.
	bool sequentialDecode = true;

	Cfg* pCfg = pProc->getCFG();

	// If this is a speculative decode, the second time we decode the same address, we get no cfg. Else an error.
	if (spec && (pCfg == 0))
		return false;
	assert(pCfg);

	// Initialise the queue of control flow targets that have yet to be decoded.
	targetQueue.initial(uAddr);

	// Clear the pointer used by the caller prologue code to access the last call rtl of this procedure
	//decoder.resetLastCall();

	// ADDRESS initAddr = uAddr;
	int nTotalBytes = 0;
	ADDRESS startAddr = uAddr;
	ADDRESS lastAddr = uAddr;

	while ((uAddr = targetQueue.nextAddress(pCfg)) != NO_ADDRESS) {

		// The list of RTLs for the current basic block
		std::list<RTL*>* BB_rtls = new std::list<RTL*>();

		// Keep decoding sequentially until a CTI without a fall through branch is decoded
		//ADDRESS start = uAddr;
		DecodeResult inst;
		while (sequentialDecode) {

			// Decode and classify the current source instruction
			if (Boomerang::get()->traceDecoder)
				LOG << "*" << uAddr << "\t";


			// Decode the inst at uAddr.
			inst = decodeInstruction(uAddr);

			// If invalid and we are speculating, just exit
			if (spec && !inst.valid)
				return false;

			// Need to construct a new list of RTLs if a basic block has just been finished but decoding is
			// continuing from its lexical successor
			if (BB_rtls == NULL)
				BB_rtls = new std::list<RTL*>();

			RTL* pRtl = inst.rtl;
			if (inst.valid == false) {
				// Alert the watchers to the problem
				Boomerang::get()->alert_baddecode(uAddr);

				// An invalid instruction. Most likely because a call did not return (e.g. call _exit()), etc.
				// Best thing is to emit a INVALID BB, and continue with valid instructions
				LOG << "Warning: invalid instruction at " << uAddr << "\n";
				// Emit the RTL anyway, so we have the address and maybe some other clues
				BB_rtls->push_back(new RTL(uAddr));	 
				pBB = pCfg->newBB(BB_rtls, INVALID, 0);
				sequentialDecode = false; BB_rtls = NULL; continue;
			}

			// alert the watchers that we have decoded an instruction
			Boomerang::get()->alert_decode(uAddr, inst.numBytes);
			nTotalBytes += inst.numBytes;			
	
			if (pRtl == 0) {
				// This can happen if an instruction is "cancelled", e.g. call to __main in a hppa program
				// Just ignore the whole instruction
				if (inst.numBytes > 0)
					uAddr += inst.numBytes;
				continue;
			}

			// Display RTL representation if asked
			if (Boomerang::get()->printRtl) {
				std::ostringstream st;
				pRtl->print(st);
				LOG << st.str().c_str();
			}
	
			ADDRESS uDest;

			// For each Statement in the RTL
			std::list<Statement*>& sl = pRtl->getList();
			std::list<Statement*>::iterator ss;
			for (ss = sl.begin(); ss != sl.end(); ss++) {
				Statement* s = *ss;
				s->setProc(pProc);		// let's do this really early!
				if (refHints.find(pRtl->getAddress()) != refHints.end()) {
					const char *nam = refHints[pRtl->getAddress()].c_str();
					ADDRESS gu = pProc->getProg()->getGlobalAddr((char*)nam);
					if (gu != NO_ADDRESS) {
						s->searchAndReplace(new Const((int)gu), new Unary(opAddrOf, Location::global(nam, pProc)));
					}
				}
				s->simplify();
				GotoStatement* stmt_jump = static_cast<GotoStatement*>(s);

				switch (s->getKind())
				{

				case STMT_GOTO: {
					uDest = stmt_jump->getFixedDest();
	
					// Handle one way jumps and computed jumps separately
					if (uDest != NO_ADDRESS) {

						BB_rtls->push_back(pRtl);
						sequentialDecode = false;

						pBB = pCfg->newBB(BB_rtls,ONEWAY,1);
						BB_rtls = NULL;		// Clear when make new BB

						// Exit the switch now if the basic block already existed
						if (pBB == 0) {
							break;
						}

						// Add the out edge if it is to a destination within the
						// procedure
						if (uDest < pBF->getLimitTextHigh()) {
							targetQueue.visit(pCfg, uDest, pBB);
							pCfg->addOutEdge(pBB, uDest, true);
						}
						else {
							LOG << "Error: Instruction at " << uAddr << " branches beyond end of section, to "
								<< uDest << "\n";
						}
					}
					break;
				}

				case STMT_CASE: {
					// MVE: check if this can happen any more
					if (stmt_jump->getDest()->getOper() == opMemOf &&
							stmt_jump->getDest()->getSubExp1()->getOper() == opIntConst && 
							pBF->IsDynamicLinkedProcPointer(((Const*)stmt_jump->getDest()->getSubExp1())->getAddr())) {
						// jump to a library function
						// replace with a call ret
						std::string func = pBF->GetDynamicProcName(
							((Const*)stmt_jump->getDest()->getSubExp1())->getAddr());
						CallStatement *call = new CallStatement;
						call->setDest(stmt_jump->getDest()->clone());
						LibProc *lp = pProc->getProg()->getLibraryProc(func.c_str());
						assert(lp);
						call->setDestProc(lp);
						std::list<Statement*>* stmt_list = new std::list<Statement*>;
						stmt_list->push_back(call);
						BB_rtls->push_back(new RTL(pRtl->getAddress(), stmt_list));
						pBB = pCfg->newBB(BB_rtls, CALL, 1);
						ReturnStatement *ret = new ReturnStatement();
						std::list<RTL*> *ret_rtls = new std::list<RTL*>();
						stmt_list = new std::list<Statement*>;
						stmt_list->push_back(ret);
						ret_rtls->push_back(new RTL(pRtl->getAddress()+1, stmt_list));
						PBB pret = pCfg->newBB(ret_rtls, RET, 0);
						pret->addInEdge(pBB);
						pBB->setOutEdge(0, pret);
						sequentialDecode = false;
						BB_rtls = NULL;
						if (pRtl->getAddress() == pProc->getNativeAddress()) {
							// it's a thunk
							// Proc *lp = prog->findProc(func.c_str());
							func = std::string("__imp_") + func;
							pProc->setName(func.c_str());
							//lp->setName(func.c_str());
							Boomerang::get()->alert_update_signature(pProc);
						}
						callSet.insert(call);
						ss = sl.end(); ss--;	// get out of the loop
						break;
					}
					BB_rtls->push_back(pRtl);
					// We create the BB as a COMPJUMP type, then change
					// to an NWAY if it turns out to be a switch stmt
					pBB = pCfg->newBB(BB_rtls, COMPJUMP, 0);
					LOG << "COMPUTED JUMP at " << uAddr << "\n";
					sequentialDecode = false;
					BB_rtls = NULL;		// New RTLList for next BB
					break;
				}


				case STMT_BRANCH: {
					uDest = stmt_jump->getFixedDest();
					BB_rtls->push_back(pRtl);
					pBB = pCfg->newBB(BB_rtls, TWOWAY, 2);

					// Stop decoding sequentially if the basic block already
					// existed otherwise complete the basic block
					if (pBB == 0)
						sequentialDecode = false;
					else {

						// Add the out edge if it is to a destination within the
						// procedure
						if (uDest < pBF->getLimitTextHigh()) {
							targetQueue.visit(pCfg, uDest, pBB);
							pCfg->addOutEdge(pBB, uDest, true);
						}
						else {
							LOG << "Error: Instruction at " << uAddr << " branches beyond end of section, to "
								<< uDest << "\n";
						}

						// Add the fall-through outedge
						pCfg->addOutEdge(pBB, uAddr + inst.numBytes); 
					}

					// Create the list of RTLs for the next basic block and
					// continue with the next instruction.
					BB_rtls = NULL;
					break;
				}

				case STMT_CALL: {
					CallStatement* call = static_cast<CallStatement*>(s);
					
					if (call->getDest()->getOper() == opMemOf &&
							call->getDest()->getSubExp1()->getOper() == opIntConst &&
							pBF->IsDynamicLinkedProcPointer(((Const*)call->getDest()->getSubExp1())->getAddr())) {
						// dynamic linked proc pointers are assumed to be static.
						const char *nam = pBF->GetDynamicProcName(
							((Const*)call->getDest()->getSubExp1())->getAddr());
						Proc *p = pProc->getProg()->getLibraryProc(nam);
						call->setDestProc(p);
						call->setIsComputed(false);
					}

					// Treat computed and static calls separately
					if (call->isComputed()) {
						BB_rtls->push_back(pRtl);
						pBB = pCfg->newBB(BB_rtls, COMPCALL, 1);

						// Stop decoding sequentially if the basic block already
						// existed otherwise complete the basic block
						if (pBB == 0)
							sequentialDecode = false;
						else
							pCfg->addOutEdge(pBB, uAddr + inst.numBytes);
						// Add this call to the list of calls to analyse. We won't
						// be able to analyse it's callee(s), of course.
						callSet.insert(call);
					}
					else {		// Static call
						// Find the address of the callee.
						ADDRESS uNewAddr = call->getFixedDest();

						// Calls with 0 offset (i.e. call the next instruction) are simply pushing the PC to the
						// stack. Treat these as non-control flow instructions and continue.
						if (uNewAddr == uAddr + inst.numBytes)
							break;

						// Call the virtual helper function. If implemented, will check for machine specific funcion
						// calls
						if (helperFunc(uNewAddr, uAddr, BB_rtls)) {
							// We have already added to BB_rtls
							pRtl = NULL;		// Discard the call semantics
							break;
						}

						BB_rtls->push_back(pRtl);

						// Add this non computed call site to the set of call sites which need to be analysed later.
						//pCfg->addCall(call);
						callSet.insert(call);

						// Record the called address as the start of a new procedure if it didn't already exist.
						if (uNewAddr && pProc->getProg()->findProc(uNewAddr) == NULL) {
							callSet.insert(call);
							//newProc(pProc->getProg(), uNewAddr);
							if (Boomerang::get()->traceDecoder)
								LOG << "p" << uNewAddr << "\t";
						}

						// Check if this is the _exit or exit function. May prevent us from attempting to decode
						// invalid instructions, and getting invalid stack height errors
						const char* name = pBF->SymbolByAddress(uNewAddr);
						if (name && ((strcmp(name, "_exit") == 0) || (strcmp(name,	"exit") == 0))) {
							// Create the new basic block
							pBB = pCfg->newBB(BB_rtls, CALL, 0);

							// Stop decoding sequentially
							sequentialDecode = false;
						}
						else {
							// Create the new basic block
							pBB = pCfg->newBB(BB_rtls, CALL, 1);

							if (call->isReturnAfterCall()) {
								// Constuct the RTLs for the new basic block
								std::list<RTL*>* rtls = new std::list<RTL*>();
								// The only RTL in the basic block is one with a ReturnStatement
								std::list<Statement*>* sl = new std::list<Statement*>;
								sl->push_back(new ReturnStatement());
								rtls->push_back(new RTL(pRtl->getAddress()+1, sl));
		
								BasicBlock* returnBB = pCfg->newBB(rtls, RET, 0);
								// Add out edge from call to return
								pCfg->addOutEdge(pBB, returnBB);
								// Put a label on the return BB (since it's an orphan); a jump will be reqd
								pCfg->setLabel(returnBB);
								pBB->setJumpReqd();
								// Mike: do we need to set return locations?
								// This ends the function
								sequentialDecode = false;
							}
							else {
								// Add the fall through edge if the block didn't
								// already exist
								if (pBB != NULL)
									pCfg->addOutEdge(pBB, uAddr+inst.numBytes);
							}
						}
					}

					// Create the list of RTLs for the next basic block and continue with the next instruction.
					BB_rtls = NULL;
					break;	
				}

				case STMT_RET: {
					// Stop decoding sequentially
					sequentialDecode = false;

#if 0
					if (pProc->getTheReturnAddr() == NO_ADDRESS) {

						// Add the RTL to the list
						BB_rtls->push_back(pRtl);
						// Create the basic block
						pBB = pCfg->newBB(BB_rtls, RET, 0);

						pProc->setTheReturnAddr((ReturnStatement*)s, pRtl->getAddress());

						// If this ret pops anything other than the return
						// address, this information can be useful in the proc
						int popped = ((ReturnStatement*)s)->getNumBytesPopped(); 
						if (popped != 0)
							// This also gives us information about the calling
							// convention
							; //pProc->setBytesPopped(popped);
					} else {
						ADDRESS retAddr = pProc->getTheReturnAddr();
						std::list<Statement*> *stmt_list = new std::list<Statement*>;
						stmt_list->push_back(new GotoStatement(retAddr));
						BB_rtls->push_back(new RTL(uAddr, stmt_list));
						targetQueue.visit(pCfg, retAddr, pBB);
						pBB = pCfg->newBB(BB_rtls, ONEWAY, 1);
						pCfg->addOutEdge(pBB, retAddr, true);
					}
#else
					pBB = createReturnBlock(pProc, BB_rtls, pRtl);
#endif

					// Create the list of RTLs for the next basic block and
					// continue with the next instruction.
					BB_rtls = NULL;		// New RTLList for next BB
				}
				break;

				case STMT_BOOLASSIGN:
					// This is just an ordinary instruction; no control transfer
					// Fall through
				case STMT_ASSIGN:
				case STMT_PHIASSIGN:
				case STMT_IMPASSIGN:
					// Do nothing
					break;
		
				} // switch (s->getKind())
			}
			if (BB_rtls && pRtl)
				// If non null, we haven't put this RTL into a the current BB as yet
				BB_rtls->push_back(pRtl);

			if (inst.reDecode)
				// Special case: redecode the last instruction, without advancing uAddr by numBytes
				continue;
			uAddr += inst.numBytes;
			if (uAddr > lastAddr)
				lastAddr = uAddr;

			// If sequentially decoding, check if the next address happens to
			// be the start of an existing BB. If so, finish off the current BB
			// (if any RTLs) as a fallthrough, and	no need to decode again
			// (unless it's an incomplete BB, then we do decode it).
			// In fact, mustn't decode twice, because it will muck up the
			// coverage, but also will cause subtle problems like add a call
			// to the list of calls to be processed, then delete the call RTL
			// (e.g. Pentium 134.perl benchmark)
			if (sequentialDecode && pCfg->existsBB(uAddr)) {
				// Create the fallthrough BB, if there are any RTLs at all
				if (BB_rtls) {
					PBB pBB = pCfg->newBB(BB_rtls, FALL, 1);
					// Add an out edge to this address
					if (pBB) {
						pCfg->addOutEdge(pBB, uAddr);
						BB_rtls = NULL;			// Need new list of RTLs
					}
				}
				// Pick a new address to decode from, if the BB is complete
				if (!pCfg->isIncomplete(uAddr))
					sequentialDecode = false;
			}

		}	// while sequentialDecode

		// Add this range to the coverage
//		  pProc->addRange(start, uAddr);

		// Must set sequentialDecode back to true
		sequentialDecode = true;

	}	// while nextAddress() != NO_ADDRESS

	//ProgWatcher *w = prog->getWatcher();
	//if (w)
	//	  w->alert_done(pProc, initAddr, lastAddr, nTotalBytes);

	// Add the callees to the set of CallStatements to process for CSR, and also to the Prog object
	std::set<CallStatement*>::iterator it;
	for (it = callSet.begin(); it != callSet.end(); it++) {
		ADDRESS dest = (*it)->getFixedDest();
		// Don't speculatively decode procs that are outside of the main text section, apart from dynamically
		// linked ones (in the .plt)
		if (pBF->IsDynamicLinkedProc(dest) || !spec || (dest < pBF->getLimitTextHigh())) {
			pCfg->addCall(*it);
			// Don't visit the destination of a register call
			Proc *np = (*it)->getDestProc();
			if (np == NULL && dest != NO_ADDRESS) {
				//np = newProc(pProc->getProg(), dest);
				np = pProc->getProg()->setNewProc(dest);
			}
			if (np != NULL) {
				np->setFirstCaller(pProc);
				pProc->addCallee(np);
			}			
		}
	}

	Boomerang::get()->alert_decode(pProc, startAddr, lastAddr, nTotalBytes);

	return true;
}

/*==============================================================================
 * FUNCTION:	FrontEnd::getInst
 * OVERVIEW:	Fetch the smallest (nop-sized) instruction, in an endianness independent manner
 * NOTE:		Frequently overridden
 * PARAMETERS:	addr - host address to getch from
 * RETURNS:		An integer with the instruction in it
 *============================================================================*/
int FrontEnd::getInst(int addr)
{
	return (int)(*(unsigned char*)addr);
}


/*==============================================================================
 * FUNCTION:	TargetQueue::visit
 * OVERVIEW:	Visit a destination as a label, i.e. check whether we need to queue it as a new BB to create later.
 *				Note: at present, it is important to visit an address BEFORE an out edge is added to that address.
 *				This is because adding an out edge enters the address into the Cfg's BB map, and it looks like the
 *				BB has already been visited, and it gets overlooked. It would be better to have a scheme whereby
 *				the order of calling these functions (i.e. visit() and AddOutEdge()) did not matter.
 * PARAMETERS:	pCfg - the enclosing CFG
 *				uNewAddr - the address to be checked
 *				pNewBB - set to the lower part of the BB if the address
 *				already exists as a non explicit label (BB has to be split)
 * RETURNS:		<nothing>
 *============================================================================*/
void TargetQueue::visit(Cfg* pCfg, ADDRESS uNewAddr, PBB& pNewBB) {
	// Find out if we've already parsed the destination
	bool bParsed = pCfg->label(uNewAddr, pNewBB);
	// Add this address to the back of the local queue,
	// if not already processed
	if (!bParsed) {
		targets.push(uNewAddr);
		if (Boomerang::get()->traceDecoder)
			LOG << ">" << uNewAddr << "\t";
	}
}

/*==============================================================================
 * FUNCTION:	TargetQueue::initial
 * OVERVIEW:	Seed the queue with an initial address
 * PARAMETERS:	uAddr: Native address to seed the queue with
 * RETURNS:		<nothing>
 *============================================================================*/
void TargetQueue::initial(ADDRESS uAddr) {
	assert(targets.empty());			// Ensure no residue from previous procedures
	targets.push(uAddr);
}

/*==============================================================================
 * FUNCTION:		  TergetQueue::nextAddress
 * OVERVIEW:		  Return the next target from the queue of non-processed
 *					  targets.
 * PARAMETERS:		  cfg - the enclosing CFG
 * RETURNS:			  The next address to process, or NO_ADDRESS if none
 *						(targets is empty)
 *============================================================================*/
ADDRESS TargetQueue::nextAddress(Cfg* cfg) {
	while (!targets.empty())
	{
		ADDRESS address = targets.front();
		targets.pop();
		if (Boomerang::get()->traceDecoder)
			LOG << "<" << address << "\t";

		// If no label there at all, or if there is a BB, it's incomplete,
		// then we can parse this address next
		if (!cfg->existsBB(address) || cfg->isIncomplete(address))
			return address;
	}
	return NO_ADDRESS;
}

/*==============================================================================
 * FUNCTION:	  decodeRtl
 * OVERVIEW:	  Decode the RTL at the given address
 * PARAMETERS:	  address - native address of the instruction
 *				  delta - difference between host and native addresses
 *				  decoder - decoder object
 * NOTE:		  Only called from findCoverage()
 * RETURNS:		  a pointer to the decoded RTL
 *============================================================================*/
RTL* decodeRtl(ADDRESS address, int delta, NJMCDecoder* decoder) {
	DecodeResult inst = 
		decoder->decodeInstruction(address, delta);

	RTL*	rtl	= inst.rtl;

	return rtl;
}


/*==============================================================================
 * FUNCTION:	getInstanceFor
 * OVERVIEW:	Guess the machine required to decode this binary file;
 *				  load the library and return an instance of FrontEnd
 * PARAMETERS:	sName: name of the binary file
 *				dlHandle: ref to a void* needed for closeInstance
 *				prog: the program to decode
 *				decoder: ref to ptr to decoder object
 * RETURNS:		Pointer to a FrontEnd* object, or 0 if not successful
 *============================================================================*/
typedef FrontEnd* (*constructFcn)(int, ADDRESS, NJMCDecoder**);
#define TESTMAGIC2(buf,off,a,b)		(buf[off] == a && buf[off+1] == b)
#define TESTMAGIC4(buf,off,a,b,c,d) (buf[off] == a && buf[off+1] == b && \
									 buf[off+2] == c && buf[off+3] == d)
FrontEnd* FrontEnd::getInstanceFor( const char *sName, void*& dlHandle, BinaryFile *pBF, NJMCDecoder*& decoder) {
	FILE *f;
	char buf[64];
	std::string libName, machName;
	dlHandle = 0;			// Only used with DYNAMIC code

	f = fopen (sName, "ro");
	if( f == NULL ) {
		LOG << "Unable to open binary file: " << sName << "\n";
		fclose(f);
		return NULL;
	}
	fread (buf, sizeof(buf), 1, f);
	fclose(f);

	if( TESTMAGIC4(buf,0, '\x7F','E','L','F') ) {
		// ELF Binary; they have an enum for the machine!
		if (buf[0x13] == 2) {		// Sparc, big endian
			machName = "sparc"; 
#ifndef DYNAMIC
			{
				SparcFrontEnd *fe = new SparcFrontEnd(pBF);
				decoder = fe->getDecoder();
				return fe;
			}
#endif
		}
		else if (buf[0x12] == 3) {
			machName = "pentium"; 
#ifndef DYNAMIC
			{
				PentiumFrontEnd *fe = new PentiumFrontEnd(pBF);
				decoder = fe->getDecoder();
				return fe;
			}
#endif
		} else {
			LOG << "Unknown ELF machine type " << (ADDRESS)buf[0x12] << (ADDRESS)buf[0x13] << "\n";
			return NULL;
		}
	} else if( TESTMAGIC2( buf,0, 'M','Z' ) ) { /* DOS-based file */
		// This test could be strengthened a bit!
		machName = "pentium";
#ifndef DYNAMIC
		PentiumFrontEnd *fe = new PentiumFrontEnd(pBF);
		decoder = fe->getDecoder();
		return fe;
#endif
	} else if( TESTMAGIC4( buf,0x3C, 'a','p','p','l' ) || TESTMAGIC4( buf,0x3C, 'p','a','n','l' ) ) {
		/* PRC Palm-pilot binary */
		machName = "mc68k";
	} else if( buf[0] == 0x02 && buf[2] == 0x01 &&
				(buf[1] == 0x10 || buf[1] == 0x0B) &&
				(buf[3] == 0x07 || buf[3] == 0x08 || buf[4] == 0x0B) ) {
		/* HP Som binary (last as it's not really particularly good magic) */
		libName = "hppa";
	} else {
		LOG << "FrontEnd::getInstanceFor: unrecognised binary file" <<
			sName << "\n";
		return NULL;
	}

#ifdef DYNAMIC
	// Load the specific decoder library
	libName = std::string("lib/libfront") + machName + ".so";
	dlHandle = dlopen(libName.c_str(), RTLD_LAZY);
	if (dlHandle == NULL) {
		LOG << "Could not open dynamic loader library " << libName << "\n";
		LOG << "dlerror is " << dlerror() << "\n";
		return NULL;
	}
	// Use the handle to find the "construct" function
	constructFcn pFcn = (constructFcn) dlsym(dlHandle, "construct");
	if (pFcn == NULL) {
		LOG << "Front end library " << libName <<
			" does not have a construct function\n";
		return NULL;
	}

	// Call the construct function
	return (*pFcn)(pBF, &decoder);
#endif

	return 0;
}

/*==============================================================================
 * FUNCTION:	FrontEnd::closeInstance
 * OVERVIEW:	Close the library opened by getInstanceFor
 * PARAMETERS:	dlHandle: a void* from getInstanceFor
 * NOTE:		static function
 * RETURNS:		Nothing
 *============================================================================*/
void FrontEnd::closeInstance(void* dlHandle) {
#ifdef DYNAMIC
	if (dlHandle) dlclose(dlHandle);
#endif
}

/*==============================================================================
 * FUNCTION:	FrontEnd::getProg
 * OVERVIEW:	Get a Prog object (mainly for testing and not decoding)
 * NOTE:		Caller to destroy
 * PARAMETERS:	None
 * RETURNS:		Pointer to a Prog object (with pFE and pBF filled in)
 *============================================================================*/
Prog* FrontEnd::getProg() {
	Prog *prog = new Prog(pBF, this);
	return prog;
}

/*==============================================================================
 * FUNCTION:	createReturnBlock
 * OVERVIEW:	Create a Return or a Oneway BB if a return statement already exists
 * PARAMETERS:	pProc: pointer to enclosing UserProc
 *				BB_rtls: list of RTLs for the current BB (not including pRtl)
 *				pRtl: pointer to the current RTL with the semantics for the return statement (including a
 *					ReturnStatement as the last statement)
 * RETURNS:		Pointer to the newly created BB
 *============================================================================*/
PBB FrontEnd::createReturnBlock(UserProc* pProc, std::list<RTL*>* BB_rtls, RTL* pRtl) {
	Cfg* pCfg = pProc->getCFG();
	PBB pBB;
	// Add the RTL to the list; this has the semantics for the return statement as well as the ReturnStatement
	// The last Statement may get replaced with a GotoStatement
	if (BB_rtls == NULL) BB_rtls = new std::list<RTL*>;		// In case no other semantics
	BB_rtls->push_back(pRtl);
	if (pProc->getTheReturnAddr() == NO_ADDRESS) {
		// Create the basic block
		pBB = pCfg->newBB(BB_rtls, RET, 0);
		Statement* s = pRtl->getList().back();		// The last statement should be the ReturnStatement
		pProc->setTheReturnAddr((ReturnStatement*)s, pRtl->getAddress());
	} else {
		ADDRESS retAddr = pProc->getTheReturnAddr();
		// Must beware of modifying *pRtl, since there is an outer loop iterating through each statement.
		// It's fine to overwrite the last Statement* though, which replaceLastStmt does
		pRtl->replaceLastStmt(new GotoStatement(retAddr));
		pBB = pCfg->newBB(BB_rtls, ONEWAY, 1);
		if (pBB)									// Can be NULL if BB already exists but is incomplete
			pCfg->addOutEdge(pBB, retAddr, true);
		// Visit the return instruction. This will be needed in most cases to split the return BB (if it has other
		// instructions before the return instruction).
		targetQueue.visit(pCfg, retAddr, pBB);
	}
	return pBB;
}
