#define sign_extend(N, SIZE) (((int)((N) << (sizeof(unsigned) * 8 - (SIZE)))) >> (sizeof(unsigned) * 8 - (SIZE)))

//#line 1 "frontend/machine/mips/decoder.m"
/****************************************************************
*
* FILENAME
*
*   \file decoder.m
*
* PURPOSE
*
* Decoding MIPS
*
* AUTHOR
*
*   \author Markus Gothe, nietzsche@lysator.liu.se
*
* REVISION
*
*   $Id$
*
*****************************************************************/

#include "mipsdecoder.h"

#include "exp.h"
#include "prog.h"
#include "proc.h"
#include "decoder.h"
#include "rtl.h"
#include "BinaryFile.h" // For SymbolByAddress()
#include "boomerang.h"
#include "statement.h"

#include <cassert>
#include <iostream>

/********************************************************************************
 * \fn       unused
 * \brief       A dummy function to suppress "unused local variable" messages
 * \param       x: integer variable to be "used"
 ********************************************************************************/
void MIPSDecoder::unused(int /*x*/) {}

MIPSDecoder::MIPSDecoder(Prog *prog) : NJMCDecoder(prog) {
    QDir base_dir=Boomerang::get()->getProgDir();
    RTLDict.readSSLFile(base_dir.absoluteFilePath("frontend/machine/mips/mips.ssl"));
}

// For now...
int MIPSDecoder::decodeAssemblyInstruction(ADDRESS, ptrdiff_t) { return 0; }

// Stub from PPC...
/****************************************************************************/ /**
  * \brief   Attempt to decode the high level instruction at a given
  *              address and return the corresponding HL type (e.g. CallStatement,
  *              GotoStatement etc). If no high level instruction exists at the
  *              given address, then simply return the RTL for the low level
  *              instruction at this address. There is an option to also
  *              include the low level statements for a HL instruction.
  * \param   pc - the native address of the pc
  * \param   delta - the difference between the above address and the
  *              host address of the pc (i.e. the address that the pc is at in the loaded object file)
  * \returns a DecodeResult structure containing all the information
  *              gathered during decoding
  *********************************************************************************/
DecodeResult &MIPSDecoder::decodeInstruction(ADDRESS pc, ptrdiff_t delta) {
    static DecodeResult result;
    // ADDRESS hostPC = pc+delta;

    // Clear the result structure;
    result.reset();

    // The actual list of instantiated statements
    // std::list<Statement*>* stmts = nullptr;
    // ADDRESS nextPC = NO_ADDRESS;
    // Decoding goes here....

    return result;
}
