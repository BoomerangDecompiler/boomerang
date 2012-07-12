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
 * $Revision$	// 1.89.2.7
 * 08 Apr 02 - Mike: Mods to adapt UQBT code to boomerang
 * 16 May 02 - Mike: Moved getMainEntry point here from prog
 * 09 Jul 02 - Mike: Fixed machine check for elf files (was checking endianness rather than machine type)
 * 22 Nov 02 - Mike: Quelched warnings
 * 16 Apr 03 - Mike: trace (-t) to cerr not cout now
 * 02 Feb 05 - Gerard: Check for thunks to library functions and don't create procs for these
 */

#include <assert.h>
#include <cstring>
#include <stdlib.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "frontend.h"
#include <queue>
#include <stdarg.h>			// For varargs
#include <sstream>
#ifndef _WIN32
#include <dlfcn.h>			// dlopen, dlsym
#endif

#include "types.h"
#include "exp.h"
#include "cfg.h"
#include "proc.h"
#include "register.h"
#include "rtl.h"
#include "BinaryFile.h"
#include "decoder.h"
#include "sparcfrontend.h"
#include "pentiumfrontend.h"
#include "ppcfrontend.h"
#include "st20frontend.h"
#include "mipsfrontend.h"
#include "prog.h"
#include "signature.h"
#include "boomerang.h"
#include "log.h"
#include "ansi-c-parser.h"

/*==============================================================================
 * FUNCTION:	  FrontEnd::FrontEnd
 * OVERVIEW:	  Construct the FrontEnd object
 * PARAMETERS:	  pBF: pointer to the BinaryFile object (loader)
 *				  prog: program being decoded
 *				  pbff: pointer to a BinaryFileFactory object (so the library can be unloaded)
 * RETURNS:		  <N/a>
 *============================================================================*/
FrontEnd::FrontEnd(BinaryFile *pBF, Prog* prog, BinaryFileFactory* pbff) : pBF(pBF), pbff(pbff), prog(prog)
{}

// Static function to instantiate an appropriate concrete front end
FrontEnd* FrontEnd::instantiate(BinaryFile *pBF, Prog* prog, BinaryFileFactory* pbff) {
	switch(pBF->GetMachine()) {
		case MACHINE_PENTIUM:
			return new PentiumFrontEnd(pBF, prog, pbff);
		case MACHINE_SPARC:
			return new SparcFrontEnd(pBF, prog, pbff);
		case MACHINE_PPC:
			return new PPCFrontEnd(pBF, prog, pbff);
		case MACHINE_MIPS:
			return new MIPSFrontEnd(pBF, prog, pbff);			
		case MACHINE_ST20:
			return new ST20FrontEnd(pBF, prog, pbff);
		default:
			std::cerr << "Machine architecture not supported!\n";
	}
	return NULL;
}

FrontEnd* FrontEnd::Load(const char *fname, Prog* prog) {
	BinaryFileFactory* pbff = new BinaryFileFactory;
	if (pbff == NULL) return NULL;
	BinaryFile *pBF = pbff->Load(fname);
	if (pBF == NULL) return NULL;
	return instantiate(pBF, prog, pbff);
}

// destructor
FrontEnd::~FrontEnd() {
	if (pbff)
		pbff->UnLoad();			// Unload the BinaryFile library with dlclose() or FreeLibrary()
}

const char *FrontEnd::getRegName(int idx) { 
	std::map<std::string, int, std::less<std::string> >::iterator it;
	for (it = decoder->getRTLDict().RegMap.begin();	 it != decoder->getRTLDict().RegMap.end(); it++)
		if ((*it).second == idx) 
			return (*it).first.c_str();
	return NULL;
}

int FrontEnd::getRegSize(int idx) {
	if (decoder->getRTLDict().DetRegMap.find(idx) == decoder->getRTLDict().DetRegMap.end())
		return 32;
	return decoder->getRTLDict().DetRegMap[idx].g_size();
}

bool FrontEnd::isWin32() {
	return pBF->GetFormat() == LOADFMT_PE;
}

bool FrontEnd::noReturnCallDest(const char *name)
{
	return ((strcmp(name, "_exit") == 0) || (strcmp(name,	"exit") == 0) || (strcmp(name, "ExitProcess") == 0) || (strcmp(name, "abort") == 0) || (strcmp(name, "_assert") == 0));
}

// FIXME: Is this ever used? Need to pass a real pbff?
FrontEnd *FrontEnd::createById(std::string &str, BinaryFile *pBF, Prog* prog) {
	if (str == "pentium")
		return new PentiumFrontEnd(pBF, prog, NULL);
	if (str == "sparc")
		return new SparcFrontEnd(pBF, prog, NULL);
	if (str == "ppc")
		return new PPCFrontEnd(pBF, prog, NULL);
	if (str == "mips")
		return new MIPSFrontEnd(pBF, prog, NULL);
	if (str == "st20")
		return new ST20FrontEnd(pBF, prog, NULL);
	return NULL;
}

void FrontEnd::readLibraryCatalog(const char *sPath) {
	std::ifstream inf(sPath);
	if (!inf.good()) {
		std::cerr << "can't open `" << sPath << "'\n";
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
	sList = Boomerang::get()->getProgPath() + "signatures/" + Signature::platformName(getFrontEndId()) + ".hs";
	readLibraryCatalog(sList.c_str());
	if (isWin32()) {
		sList = Boomerang::get()->getProgPath() + "signatures/win32.hs";
		readLibraryCatalog(sList.c_str());
	}
}

std::vector<ADDRESS> FrontEnd::getEntryPoints()
{
	std::vector<ADDRESS> entrypoints;
	bool gotMain = false;
	ADDRESS a = getMainEntryPoint(gotMain);
	if (a != NO_ADDRESS)
		entrypoints.push_back(a);
	else {  // try some other tricks
		const char *fname = pBF->getFilename();
		// X11 Module
		if (!strcmp(fname + strlen(fname) - 6, "_drv.o")) {
			const char *p = fname + strlen(fname) - 6;
			while (*p != '/' && *p != '\\' && p != fname)
				p--;
			if (p != fname) {
				p++;
				char *name = (char*)malloc(strlen(p) + 30);
				strcpy(name, p);
				name[strlen(name)-6] = 0;
				strcat(name, "ModuleData");
				ADDRESS a = pBF->GetAddressByName(name, true);
				if (a != NO_ADDRESS) {
					ADDRESS vers, setup, teardown;
					vers = pBF->readNative4(a);
					setup = pBF->readNative4(a+4);
					teardown = pBF->readNative4(a+8);
					if (setup) {
						Type *ty = NamedType::getNamedType("ModuleSetupProc");
						assert(ty->isFunc());
						UserProc *proc = (UserProc*)prog->setNewProc(setup);
						assert(proc);
						Signature *sig = ty->asFunc()->getSignature()->clone();
						const char *sym = pBF->SymbolByAddress(setup);
						if (sym)
							sig->setName(sym);
						sig->setForced(true);
						proc->setSignature(sig);
						entrypoints.push_back(setup);
					}
					if (teardown) {
						Type *ty = NamedType::getNamedType("ModuleTearDownProc");
						assert(ty->isFunc());
						UserProc *proc = (UserProc*)prog->setNewProc(teardown);
						assert(proc);
						Signature *sig = ty->asFunc()->getSignature()->clone();
						const char *sym = pBF->SymbolByAddress(teardown);
						if (sym)
							sig->setName(sym);
						sig->setForced(true);
						proc->setSignature(sig);						
						entrypoints.push_back(teardown);
					}
				}
			}
		}
		// Linux kernel module
		if (!strcmp(fname + strlen(fname) - 3, ".ko")) {
			a = pBF->GetAddressByName("init_module");
			if (a != NO_ADDRESS)
				entrypoints.push_back(a);
			a = pBF->GetAddressByName("cleanup_module");
			if (a != NO_ADDRESS)
				entrypoints.push_back(a);
		}
	}
	return entrypoints;
}

void FrontEnd::decode(Prog* prog, bool decodeMain, const char *pname) {
	if (pname)
		prog->setName(pname);

	if (!decodeMain)
		return;
	
	Boomerang::get()->alert_start_decode(pBF->getLimitTextLow(), pBF->getLimitTextHigh() - pBF->getLimitTextLow());

	bool gotMain;
	ADDRESS a = getMainEntryPoint(gotMain);
	if (VERBOSE)
		LOG << "start: " << a << " gotmain: " << (gotMain ? "true" : "false") << "\n";
	if (a == NO_ADDRESS) {
		std::vector<ADDRESS> entrypoints = getEntryPoints();
		for (std::vector<ADDRESS>::iterator it = entrypoints.begin(); it != entrypoints.end(); it++)
			decode(prog, *it);
		return;
	}

	decode(prog, a);
	prog->setEntryPoint(a);

	if (gotMain) {
		static const char *mainName[] = { "main", "WinMain", "DriverEntry" };
		const char *name = pBF->SymbolByAddress(a);
		if (name == NULL)
			name = mainName[0];
		for (size_t i = 0; i < sizeof(mainName)/sizeof(char*); i++) {
			if (!strcmp(name, mainName[i])) {
				Proc *proc = prog->findProc(a);
				if (proc == NULL) {
					if (VERBOSE)
						LOG << "no proc found for address " << a << "\n";
					return;
				}
				FuncType *fty = dynamic_cast<FuncType*>(Type::getNamedType(name));
				if (fty == NULL)
					LOG << "unable to find signature for known entrypoint " << name << "\n";
				else {
					proc->setSignature(fty->getSignature()->clone());
					proc->getSignature()->setName(name);
					//proc->getSignature()->setFullSig(true);		// Don't add or remove parameters
					proc->getSignature()->setForced(true);			// Don't add or remove parameters
				}
				break;
			}
		}
	}
	return;
}

// Somehow, a == NO_ADDRESS has come to mean decode anything not already decoded
void FrontEnd::decode(Prog *prog, ADDRESS a) {
	if (a != NO_ADDRESS) {
		prog->setNewProc(a);
		if (VERBOSE)
			LOG << "starting decode at address " << a << "\n";
		UserProc* p = (UserProc*)prog->findProc(a);
		if (p == NULL) {
			if (VERBOSE)
				LOG << "no proc found at address " << a << "\n";
			return;
		}
		if (p->isLib()) {
			LOG << "NOT decoding library proc at address 0x" << a << "\n";
			return;
		}
		std::ofstream os;
		processProc(a, p, os);
		p->setDecoded();

	} else {						// a == NO_ADDRESS
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
	if (Boomerang::get()->traceDecoder)
		LOG << "decoding fragment at 0x" << a << "\n";
	std::ofstream os;
	processProc(a, proc, os, true);
}

DecodeResult& FrontEnd::decodeInstruction(ADDRESS pc) {
	if (pBF->GetSectionInfoByAddr(pc) == NULL) {
		LOG << "ERROR: attempted to decode outside any known segment " << pc << "\n";
		static DecodeResult invalid;
		invalid.reset();
		invalid.valid = false;
		return invalid;
	}
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
		std::cerr << "can't open `" << sPath << "'\n";
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
		(*it)->setSigFile(sPath);
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
		LOG << "Unknown library function " << name << "\n";
		signature = getDefaultSignature(name);
	}
	else {
		// Don't clone here; cloned in CallStatement::setSigArguments
		signature = (*it).second;
		signature->setUnknown(false);
	}
	return signature;
}

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

	// just in case you missed it
	Boomerang::get()->alert_new(pProc);

	// We have a set of CallStatement pointers. These may be disregarded if this is a speculative decode
	// that fails (i.e. an illegal instruction is found). If not, this set will be used to add to the set of calls
	// to be analysed in the cfg, and also to call newProc()
	std::list<CallStatement*> callList;

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
				if (VERBOSE) {
					LOG << "Warning: invalid instruction at " << uAddr << ": ";
					// Emit the next 4 bytes for debugging
					for (int ii=0; ii < 4; ii++)
						LOG << (ADDRESS) (pBF->readNative1(uAddr + ii) & 0xFF) << " ";
					LOG << "\n";
				}
				// Emit the RTL anyway, so we have the address and maybe some other clues
				BB_rtls->push_back(new RTL(uAddr));	 
				pBB = pCfg->newBB(BB_rtls, INVALID, 0);
				sequentialDecode = false; BB_rtls = NULL; continue;
			}

			// alert the watchers that we have decoded an instruction
			Boomerang::get()->alert_decode(uAddr, inst.numBytes);
			nTotalBytes += inst.numBytes;			
	
			// Check if this is an already decoded jump instruction (from a previous pass with propagation etc)
			// If so, we throw away the just decoded RTL (but we still may have needed to calculate the number
			// of bytes.. ick.)
			std::map<ADDRESS, RTL*>::iterator ff = previouslyDecoded.find(uAddr);
			if (ff != previouslyDecoded.end())
				pRtl = ff->second;

			if (pRtl == NULL) {
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
			//std::list<Statement*>& sl = pRtl->getList();
			std::list<Statement*> sl = pRtl->getList();
			// Make a copy (!) of the list. This is needed temporarily to work around the following problem.
			// We are currently iterating an RTL, which could be a return instruction. The RTL is passed to
			// createReturnBlock; if this is not the first return statement, it will get cleared, and this will
			// cause problems with the current iteration. The effects seem to be worse for MSVC/Windows.
			// This problem will likely be easier to cope with when the RTLs are removed, and there are special
			// Statements to mark the start of instructions (and their native address).
			// FIXME: However, this workaround breaks logic below where a GOTO is changed to a CALL followed by a return
			// if it points to the start of a known procedure
			std::list<Statement*>::iterator ss;
#if 1
			for (ss = sl.begin(); ss != sl.end(); ss++) { // }
#else
			// The counter is introduced because ss != sl.end() does not work as it should
			// FIXME: why? Does this really fix the problem?
			int counter = sl.size();
			for (ss = sl.begin(); counter > 0; ss++, counter--) {
#endif
				Statement* s = *ss;
				s->setProc(pProc);		// let's do this really early!
				if (refHints.find(pRtl->getAddress()) != refHints.end()) {
					const char *nam = refHints[pRtl->getAddress()].c_str();
					ADDRESS gu = prog->getGlobalAddr((char*)nam);
					if (gu != NO_ADDRESS) {
						s->searchAndReplace(new Const((int)gu), new Unary(opAddrOf, Location::global(nam, pProc)));
					}
				}
				s->simplify();
				GotoStatement* stmt_jump = static_cast<GotoStatement*>(s);

				// Check for a call to an already existing procedure (including self recursive jumps), or to the PLT
				// (note that a LibProc entry for the PLT function may not yet exist)
				ADDRESS dest;
				Proc* proc;
				if (s->getKind() == STMT_GOTO) {
					dest = stmt_jump->getFixedDest();
					if (dest != NO_ADDRESS) {
						proc = prog->findProc(dest);
						if (proc == NULL) {
							if (pBF->IsDynamicLinkedProc(dest))
								proc = prog->setNewProc(dest);
						}
						if (proc != NULL && proc != (Proc*)-1) {
							s = new CallStatement();
							CallStatement *call = static_cast<CallStatement*>(s);
							call->setDest(dest);
							call->setDestProc(proc);
							call->setReturnAfterCall(true);
							// also need to change it in the actual RTL
							std::list<Statement*>::iterator ss1 = ss;
							ss1++;
							assert(ss1 == sl.end());
							pRtl->replaceLastStmt(s);
							*ss = s;
						}
					}
				}

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
					Exp* pDest = stmt_jump->getDest();
					if (pDest == NULL) {				// Happens if already analysed (now redecoding)
						// SWITCH_INFO* psi = ((CaseStatement*)stmt_jump)->getSwitchInfo();
						BB_rtls->push_back(pRtl);
						pBB = pCfg->newBB(BB_rtls, NWAY, 0);	// processSwitch will update num outedges
						pBB->processSwitch(pProc);		// decode arms, set out edges, etc
						sequentialDecode = false;		// Don't decode after the jump
						BB_rtls = NULL;					// New RTLList for next BB
						break;							// Just leave it alone
					}
					// Check for indirect calls to library functions, especially in Win32 programs
					if (pDest && pDest->getOper() == opMemOf &&
							pDest->getSubExp1()->getOper() == opIntConst && 
							pBF->IsDynamicLinkedProcPointer(((Const*)pDest->getSubExp1())->getAddr())) {
						if (VERBOSE)
							LOG << "jump to a library function: " << stmt_jump << ", replacing with a call/ret.\n";
						// jump to a library function
						// replace with a call ret
						std::string func = pBF->GetDynamicProcName(
							((Const*)stmt_jump->getDest()->getSubExp1())->getAddr());
						CallStatement *call = new CallStatement;
						call->setDest(stmt_jump->getDest()->clone());
						LibProc *lp = pProc->getProg()->getLibraryProc(func.c_str());
						if (lp == NULL)
							LOG << "getLibraryProc returned NULL, aborting\n";
						assert(lp);
						call->setDestProc(lp);
						std::list<Statement*>* stmt_list = new std::list<Statement*>;
						stmt_list->push_back(call);
						BB_rtls->push_back(new RTL(pRtl->getAddress(), stmt_list));
						pBB = pCfg->newBB(BB_rtls, CALL, 1);
						appendSyntheticReturn(pBB, pProc, pRtl);
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
						callList.push_back(call);
						ss = sl.end(); ss--;	// get out of the loop
						break;
					}
					BB_rtls->push_back(pRtl);
					// We create the BB as a COMPJUMP type, then change to an NWAY if it turns out to be a switch stmt
					pBB = pCfg->newBB(BB_rtls, COMPJUMP, 0);
					LOG << "COMPUTED JUMP at " << uAddr << ", pDest = " << pDest << "\n";
					if (Boomerang::get()->noDecompile) {
						// try some hacks
						if (pDest->isMemOf() && pDest->getSubExp1()->getOper() == opPlus &&
								pDest->getSubExp1()->getSubExp2()->isIntConst()) {
							// assume subExp2 is a jump table
							ADDRESS jmptbl = ((Const*)pDest->getSubExp1()->getSubExp2())->getInt();
							unsigned int i;
							for (i = 0; ; i++) {
								ADDRESS uDest = pBF->readNative4(jmptbl + i * 4);
								if (pBF->getLimitTextLow() <= uDest && uDest < pBF->getLimitTextHigh()) {
									LOG << "  guessed uDest " << uDest << "\n";
									targetQueue.visit(pCfg, uDest, pBB);
									pCfg->addOutEdge(pBB, uDest, true);
								} else
									break;
							}
							pBB->updateType(NWAY, i);
						}
					}
					sequentialDecode = false;
					BB_rtls = NULL;		// New RTLList for next BB
					break;
				}


				case STMT_BRANCH: {
					uDest = stmt_jump->getFixedDest();
					BB_rtls->push_back(pRtl);
					pBB = pCfg->newBB(BB_rtls, TWOWAY, 2);

					// Stop decoding sequentially if the basic block already existed otherwise complete the basic block
					if (pBB == 0)
						sequentialDecode = false;
					else {

						// Add the out edge if it is to a destination within the procedure
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

					// Create the list of RTLs for the next basic block and continue with the next instruction.
					BB_rtls = NULL;
					break;
				}

				case STMT_CALL: {
					CallStatement* call = static_cast<CallStatement*>(s);
					
					// Check for a dynamic linked library function
					if (call->getDest()->getOper() == opMemOf &&
							call->getDest()->getSubExp1()->getOper() == opIntConst &&
							pBF->IsDynamicLinkedProcPointer(((Const*)call->getDest()->getSubExp1())->getAddr())) {
						// Dynamic linked proc pointers are treated as static.
						const char *nam = pBF->GetDynamicProcName( ((Const*)call->getDest()->getSubExp1())->getAddr());
						Proc *p = pProc->getProg()->getLibraryProc(nam);
						call->setDestProc(p);
						call->setIsComputed(false);
					}

					// Is the called function a thunk calling a library function?
					// A "thunk" is a function which only consists of: "GOTO library_function"
					if(	call &&	call->getFixedDest() != NO_ADDRESS ) {
						// Get the address of the called function.
						ADDRESS callAddr=call->getFixedDest();
						// It should not be in the PLT either, but getLimitTextHigh() takes this into account
						if (callAddr < pBF->getLimitTextHigh()) {
							// Decode it.
							DecodeResult decoded=decodeInstruction(callAddr);
							if (decoded.valid) { // is the instruction decoded succesfully?
								// Yes, it is. Create a Statement from it.
								RTL *rtl = decoded.rtl;
								Statement* first_statement = *rtl->getList().begin();
								if (first_statement) {
									first_statement->setProc(pProc);
									first_statement->simplify();
									GotoStatement* stmt_jump = static_cast<GotoStatement*>(first_statement);
									// In fact it's a computed (looked up) jump, so the jump seems to be a case
									// statement.
									if ( first_statement->getKind() == STMT_CASE &&
										stmt_jump->getDest()->getOper() == opMemOf &&
										stmt_jump->getDest()->getSubExp1()->getOper() == opIntConst &&
										pBF->IsDynamicLinkedProcPointer(((Const*)stmt_jump->getDest()->getSubExp1())->
											getAddr())) // Is it an "DynamicLinkedProcPointer"?
									{
										// Yes, it's a library function. Look up it's name.
										ADDRESS a = ((Const*)stmt_jump->getDest()->getSubExp1())->getAddr();
										const char *nam = pBF->GetDynamicProcName(a);
										// Assign the proc to the call
										Proc *p = pProc->getProg()->getLibraryProc(nam);
										if (call->getDestProc()) {
											// prevent unnecessary __imp procs
											prog->removeProc(call->getDestProc()->getName());
										}
										call->setDestProc(p);
										call->setIsComputed(false);
										call->setDest(Location::memOf(new Const(a)));
									}
								}
							}
						}
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
						callList.push_back(call);
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
						callList.push_back(call);

						// Record the called address as the start of a new procedure if it didn't already exist.
						if (uNewAddr && uNewAddr != NO_ADDRESS && pProc->getProg()->findProc(uNewAddr) == NULL) {
							callList.push_back(call);
							//newProc(pProc->getProg(), uNewAddr);
							if (Boomerang::get()->traceDecoder)
								LOG << "p" << uNewAddr << "\t";
						}

 						// Check if this is the _exit or exit function. May prevent us from attempting to decode
						// invalid instructions, and getting invalid stack height errors
						const char* name = pBF->SymbolByAddress(uNewAddr);
						if (name == NULL && call->getDest()->isMemOf() && 
											call->getDest()->getSubExp1()->isIntConst()) {
							ADDRESS a = ((Const*)call->getDest()->getSubExp1())->getInt();
							if (pBF->IsDynamicLinkedProcPointer(a))
								name = pBF->GetDynamicProcName(a);
						}	
						if (name && noReturnCallDest(name)) {
							// Make sure it has a return appended (so there is only one exit from the function)
							//call->setReturnAfterCall(true);		// I think only the Sparc frontend cares
							// Create the new basic block
							pBB = pCfg->newBB(BB_rtls, CALL, 1);
							appendSyntheticReturn(pBB, pProc, pRtl);

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

					extraProcessCall(call, BB_rtls);

					// Create the list of RTLs for the next basic block and continue with the next instruction.
					BB_rtls = NULL;
					break;	
				}

				case STMT_RET: {
					// Stop decoding sequentially
					sequentialDecode = false;

					pBB = createReturnBlock(pProc, BB_rtls, pRtl);

					// Create the list of RTLs for the next basic block and
					// continue with the next instruction.
					BB_rtls = NULL;		// New RTLList for next BB
				}
				break;

				case STMT_BOOLASSIGN:
					// This is just an ordinary instruction; no control transfer
					// Fall through
				case STMT_JUNCTION:
					// FIXME: Do we need to do anything here?
				case STMT_ASSIGN:
				case STMT_PHIASSIGN:
				case STMT_IMPASSIGN:
				case STMT_IMPREF:
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

			// If sequentially decoding, check if the next address happens to be the start of an existing BB. If so,
			// finish off the current BB (if any RTLs) as a fallthrough, and no need to decode again (unless it's an
			// incomplete BB, then we do decode it).
			// In fact, mustn't decode twice, because it will muck up the coverage, but also will cause subtle problems
			// like add a call to the list of calls to be processed, then delete the call RTL (e.g. Pentium 134.perl
			// benchmark)
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

	// Add the callees to the set of CallStatements, and also to the Prog object
	std::list<CallStatement*>::iterator it;
	for (it = callList.begin(); it != callList.end(); it++) {
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

	if (VERBOSE)
		LOG << "finished processing proc " << pProc->getName() << " at address " << pProc->getNativeAddress() << "\n";

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
 * NOTE:		Can be some targets already in the queue now
 * PARAMETERS:	uAddr: Native address to seed the queue with
 * RETURNS:		<nothing>
 *============================================================================*/
void TargetQueue::initial(ADDRESS uAddr) {
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

		// If no label there at all, or if there is a BB, it's incomplete, then we can parse this address next
		if (!cfg->existsBB(address) || cfg->isIncomplete(address))
			return address;
	}
	return NO_ADDRESS;
}

void TargetQueue::dump() {
	std::queue<ADDRESS> copy(targets);
	while (!copy.empty()) {
		ADDRESS a = copy.front();
		copy.pop();
		std::cerr << std::hex << a << ", ";
	}
	std::cerr << std::dec << "\n";
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
 * FUNCTION:	FrontEnd::getProg
 * OVERVIEW:	Get a Prog object (mainly for testing and not decoding)
 * PARAMETERS:	None
 * RETURNS:		Pointer to a Prog object (with pFE and pBF filled in)
 *============================================================================*/
Prog* FrontEnd::getProg() {
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
	// Add the RTL to the list; this has the semantics for the return instruction as well as the ReturnStatement
	// The last Statement may get replaced with a GotoStatement
	if (BB_rtls == NULL) BB_rtls = new std::list<RTL*>;		// In case no other semantics
	BB_rtls->push_back(pRtl);
	ADDRESS retAddr = pProc->getTheReturnAddr();
	// LOG << "retAddr = " << retAddr << " rtl = " << pRtl->getAddress() << "\n";
	if (retAddr == NO_ADDRESS) {
		// Create the basic block
		pBB = pCfg->newBB(BB_rtls, RET, 0);
		Statement* s = pRtl->getList().back();		// The last statement should be the ReturnStatement
		pProc->setTheReturnAddr((ReturnStatement*)s, pRtl->getAddress());
	} else {
		// We want to replace the *whole* RTL with a branch to THE first return's RTL. There can sometimes be extra
		// semantics associated with a return (e.g. Pentium return adds to the stack pointer before setting %pc and
		// branching). Other semantics (e.g. SPARC returning a value as part of the restore instruction) are assumed to
		// appear in a previous RTL. It is assumed that THE return statement will have the same semantics (NOTE: may
		// not always be valid). To avoid this assumption, we need branches to statements, not just to native addresses
		// (RTLs).
		PBB retBB = pProc->getCFG()->findRetNode();
		assert(retBB);
		if (retBB->getFirstStmt()->isReturn()) {
			// ret node has no semantics, clearly we need to keep ours
			pRtl->deleteLastStmt();
		} else
			pRtl->clear();
		pRtl->appendStmt(new GotoStatement(retAddr));
		try {
			pBB = pCfg->newBB(BB_rtls, ONEWAY, 1);
			// if BB already exists but is incomplete, exception is thrown
			pCfg->addOutEdge(pBB, retAddr, true);
			// Visit the return instruction. This will be needed in most cases to split the return BB (if it has other
			// instructions before the return instruction).
			targetQueue.visit(pCfg, retAddr, pBB);
		} catch(Cfg::BBAlreadyExistsError &) {
			if (VERBOSE)
				LOG << "not visiting " << retAddr << " due to exception\n";
		}
	}
	return pBB;
}

// Add a synthetic return instruction (or branch to the existing return instruction).
// NOTE: the call BB should be created with one out edge (the return or branch BB)
void FrontEnd::appendSyntheticReturn(PBB pCallBB, UserProc* pProc, RTL* pRtl) {
	ReturnStatement *ret = new ReturnStatement();
	std::list<RTL*> *ret_rtls = new std::list<RTL*>();
	std::list<Statement*>* stmt_list = new std::list<Statement*>;
	stmt_list->push_back(ret);
	PBB pret = createReturnBlock(pProc, ret_rtls, new RTL(pRtl->getAddress()+1, stmt_list));
	pret->addInEdge(pCallBB);
	pCallBB->setOutEdge(0, pret);
}
