#include <iostream>
#include "prog.h"
#include "hllcode.h"
#include "codegen/chllcode.h"

Prog prog;		// The dreaded global

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "usage: Boomerang <program>" << std::endl;
		return 1;
	}
	std::cerr << "loading..." << std::endl;
	if (!prog.LoadBinary(argv[1])) {
		std::cerr << "failed." << std::endl;
		return 1;
	}
	std::cerr << "decoding..." << std::endl;
	prog.decode();
	std::cerr << "generating code..." << std::endl;
	PROGMAP::const_iterator it;
	for (Proc *pProc = prog.getFirstProc(it); pProc; pProc = prog.getNextProc(it)) {
		if (pProc->isLib()) continue;
		UserProc *p = (UserProc*)pProc;
		if (p->isDecoded()) {
			CHLLCode code(p);
			if (p->generateCode(code)) {
				std::string str;
				code.toString(str);
	
				std::cout << "Proc: " << p->getName() << std::endl;
				std::cout << str << std::endl;
			}
		}
	}
	
	return 0;
}
