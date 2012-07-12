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

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"
#endif

#include "exp.h"
#include "prog.h"
#include "proc.h"
#include "decoder.h"
#include "mipsdecoder.h"
#include "rtl.h"
#include "BinaryFile.h"		// For SymbolByAddress()
#include "boomerang.h"
#include "statement.h"
#include <iostream>


/********************************************************************************
 * FUNCTION:	   unused
 * OVERVIEW:	   A dummy function to suppress "unused local variable" messages
 * PARAMETERS:	   x: integer variable to be "used"
 * RETURNS:		   Nothing
 ********************************************************************************/
void MIPSDecoder::unused(int x)
{}

/********************************************************************************
 * FUNCTION:       MIPSDecoder::MIPSDecoder
 * OVERVIEW:       
 * PARAMETERS:     None
 * RETURNS:                N/A
 *********************************************************************************/
MIPSDecoder::MIPSDecoder(Prog* prog) : NJMCDecoder(prog)
{
  std::string file = Boomerang::get()->getProgPath() + "frontend/machine/mips/mips.ssl";
  RTLDict.readSSLFile(file.c_str());
}

// For now...
int MIPSDecoder::decodeAssemblyInstruction(ADDRESS, int)
{ return 0; }

/********************************************************************************
 * FUNCTION:	   MIPSDecoder::decodeInstruction
 * OVERVIEW:	   Attempt to decode the high level instruction at a given
 *				   address and return the corresponding HL type (e.g. CallStatement,
 *				   GotoStatement etc). If no high level instruction exists at the
 *				   given address, then simply return the RTL for the low level
 *				   instruction at this address. There is an option to also
 *				   include the low level statements for a HL instruction.
 * PARAMETERS:	   pc - the native address of the pc
 *				   delta - the difference between the above address and the
 *					 host address of the pc (i.e. the address that the pc is at
 *					 in the loaded object file)
 *				   proc - the enclosing procedure. This can be NULL for
 *					 those of us who are using this method in an interpreter
 * RETURNS:		   a DecodeResult structure containing all the information
 *					 gathered during decoding
 *********************************************************************************/

// Stub from PPC...
DecodeResult& MIPSDecoder::decodeInstruction(ADDRESS pc, int delta)
{ 
static DecodeResult result;
ADDRESS hostPC = pc+delta;

// Clear the result structure;
result.reset();

// The actual list of instantiated statements
std::list<Statement*>* stmts = NULL;

ADDRESS nextPC = NO_ADDRESS;

}
