#include <iostream>
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "hllcode.h"
#include "codegen/chllcode.h"

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "usage: Boomerang <program>" << std::endl;
		return 1;
	}
	std::cerr << "loading..." << std::endl;
	FrontEnd *fe = FrontEnd::Load(argv[1]);
	if (fe == NULL) {
		std::cerr << "failed." << std::endl;
		return 1;
	}
	std::cerr << "decoding..." << std::endl;
	Prog *prog = fe->decode();
	std::cerr << "analysing..." << std::endl;
	prog->analyse();
	std::cerr << "decompiling..." << std::endl;
	prog->decompile();
        std::cerr << "printing..." << std::endl;
        prog->print(std::cout, true);

/*	std::cerr << "generating code..." << std::endl;
	PROGMAP::const_iterator it;
	for (Proc *pProc = prog->getFirstProc(it); pProc; pProc = prog->getNextProc(it)) {
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
*/	
	return 0;
}
