#include <iostream>
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "hllcode.h"
#include "codegen/chllcode.h"
#include "boomerang.h"

Boomerang *Boomerang::boomerang = NULL;

Boomerang::Boomerang()
{
}

int Boomerang::commandLine(int argc, const char **argv)
{
    if (argc < 2) {
        std::cerr << "usage: Boomerang <program>" << std::endl;
	return 1;
    }
    progPath = argv[0];
    // Chop off after the last slash
    size_t j = progPath.rfind("/");
    if (j != -1)
    {
        // Do the chop; keep the trailing slash
        progPath = progPath.substr(0, j+1);
    }
    else {
        std::cerr << "? No slash in argv[0]!" << std::endl;
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
    std::cerr << "generating code..." << std::endl;
    prog->generateCode(std::cout);

	return 0;
}

