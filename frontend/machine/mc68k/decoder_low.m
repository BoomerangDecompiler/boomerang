/*
 * Copyright (C) 1996, Princeton University or Owen Braun ??????
 * Copyright (C) 2000, Sun Microsystems, Inc
 * Copyright (C) 2000, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       decoder_low.m
 * OVERVIEW:   Implementation of the low level mc68000 specific parts of the
 *             NJMCDecoder class.
 *============================================================================*/

/* $Revision$
 * $Id$
 * Created by Cristina 04 Feb 2000
 *
 * Based on: 
 *  m68k__assembly.m
 *  written by Owen Braun
 *  ocbraun@princeton.edu
 *  created 4/8/96 3:57 am
 *
 *  M68kInstr method decodeInstr which uses Toolkit matching statement
 *  to decode instruction and generate _assembly-language representation
 *
 * 04 Feb 00 - Integration with UQBT's .prc loader (PalmBinaryFile)
 * 07 Feb 00 - Mike: Made numInstrWords a reference, and initialised to 1
 * 09 Feb 00 - Cristina: changed register display syntax to conform to 
 *      Motorola's assembly syntax.
 *      unlk is now a non factored constructor
 *      Added system trap function name support 
 * 11 Feb 00 - Mike: started making RTL version from disassembler
 * 13 Feb 00 - Cristina: continued 
 * 15 Feb 00 - Mike: inserted addressing mode code
 * 21 Feb 00 - Mike: support for -(an) and (an)+ (bump and bumpr)
 * 22 Feb 00 - Cristina: for ADDQ and SUBQ we need 5 chars to match SSL name
 * 25 Feb 00 - Mike: Fixed move (a7)+,d3 (was faulting)
 * 24 Mar 00 - Mike: Converted sizes to bits
 * 27 Mar 00 - Mike: Fixed instructions like addaw.ex that were not having
 *                the .ex removed before 2nd last char (using chopBoth())
 * 07 Apr 00 - Mike: Fixed semantics of movew d3, a0 (sign extends)
 * 13 Apr 00 - Mike: Fixed side effects of the above; was putting the BUMP
 *                at the wrong place
 * 22 Mar 01 - Mike: Fixed a fault when decoding mem to mem move; initialisation
 *                of RTs to an empty list was in the wrong place
 * 01 Aug 01 - Mike: Added some #includes; would not compile without these
 */

#include <assert.h>
#include <stdio.h>
#include "global.h" 
#include "decoder.h"
#include "ss.h"
#include "rtl.h"

// File scope globals
static bool IsTrap = false;
static bool prevIsTrap = false;
static char sslName[20];            // Modifiable; [name] from the toolkit is
                                    // in the text section and read-only
static int temp1 = 0;               // id of a temp; initialised on first use

/*==============================================================================
 * FUNCTION:        getWord
 * OVERVIEW:        Returns the word starting at the given address.
 * PARAMETERS:      lc - host address at which to decode 
 * RETURNS:         the decoded double
 *============================================================================*/
SWord getWord (unsigned lc)
/* get2Bytes - returns next 2-Byte from image pointed to by lc.
   Fetch in a big-endian manner  */
{
    return (SWord)((*(Byte *)lc << 8) + *(Byte *)(lc+1));
}


/*==============================================================================
 * FUNCTION:        decodeTrapName
 * OVERVIEW:        Places the name of the system call in _assembly.
 * PARAMETERS:      pc - address at which to decode 
 * RETURNS:         nil
 *============================================================================*/
void decodeTrapName (ADDRESS pc)
{
// Need to think about what to do here
    //fprintf (stderr, "dc.w #%d\t// ", (SWord)(*(SWord *)pc));  
    //fprintf (stderr, "%s", trapNames[*(SWord *)pc - sysTrapBase]); 
}

// Convert names like addb.ex, orl, mulu to appropriate SSL form
// Puts resultant name into static (file scope) global sslName
void chopDotex(char* name)
{
    int l = strlen(name);
    strcpy(sslName, name);
    if (name[l-3] == '.')           // Has .ex
        l -= 3;                     // Discard the .ex
    sslName[l] = '\0';
}

// Chop 2nd last character from a name, e.g. roxrib -> roxrb
void chop2ndLast(char* name)
{
    int l = strlen(name);
    strcpy(sslName, name);
    sslName[l-2] = name[l-1];
    sslName[l-1] = '\0';
}

// Chop .ex (if present) AND 2nd last character from a name
// e.g. roxrib.ex -> roxrb
void chopBoth(char* name)
{
    int l = strlen(name);
    strcpy(sslName, name);
    if (name[l-3] == '.')           // Has .ex
        l -= 3;                     // Discard the .ex
    sslName[l] = '\0';
    sslName[l-2] = name[l-1];
    sslName[l-1] = '\0';
}

/*==============================================================================
 * FUNCTION:       bumpRegister
 * OVERVIEW:       Add an RTAssgn to bump a register by an amount
 * PARAMETERS:     RTs: list of RTs to add this RTAssgn to
 *                 bump: amount to add to the register
 *                 bumpr: register to bump
 * RETURNS:        <nothing>
 *============================================================================*/
void bumpRegister(list<RT*>* RTs, int bump, int bumpr)
{
    // Lhs: r[bumpr]
    SemStr* lhs = new SemStr(32);
    lhs->push(idRegOf); lhs->push(idIntConst); lhs->push(bumpr);
    // Rhs: r[bumpr] + bump
    SemStr* rhs = new SemStr(32);
    rhs->push(idPlus);
    rhs->push(idRegOf); rhs->push(idIntConst); rhs->push(bumpr);
    rhs->push(idIntConst); rhs->push(bump);
    RTAssgn* pRT = new RTAssgn(lhs, rhs, 32);
    RTs->push_back(pRT);
}

/*==============================================================================
 * FUNCTION:       assignTemp
 * OVERVIEW:       Add an RTAssgn to assign src to temp1
 * PARAMETERS:     src: Pointer to SemStr for src
 *                 size: size of the assignment, in bits
 *                 temp: this function sets this parameter to point to a copy
 *                  of a SemStr that represents "temp1"
 * RETURNS:        Pointer to the assignment RT
 *============================================================================*/
RT* assignTemp(SemStr* src, int size, SemStr*& temp)
{
    if (temp1 == 0) temp1 = theSemTable.addItem("temp1");
    // Lhs: r[temp1]
    SemStr* lhs = new SemStr(size);
    lhs->push(idRegOf); lhs->push(idTemp); lhs->push(temp1);
    // Rhs is just src
    RTAssgn* pRT = new RTAssgn(lhs, src, size);
    temp = new SemStr(*lhs);
    return pRT;
}

/*==============================================================================
 * FUNCTION:       sgnExTemp
 * OVERVIEW:       Add an RTAssgn to sign extend src to temp1
 * PARAMETERS:     dest: Pointer to SemStr for dest
 *                 size: size of the source, in bits
 *                 size2: size of the destination, in bits
 *                 temp: this function sets this parameter to point to a copy
 *                  of a SemStr that represents "temp1"
 * RETURNS:        Pointer to the assignment RT
 *============================================================================*/
RT* sgnExTemp(SemStr* src, int size, int size2, SemStr*& temp)
{
    if (temp1 == 0) temp1 = theSemTable.addItem("temp1");
    // Lhs: r[temp1]
    SemStr* lhs = new SemStr(size2);
    lhs->push(idRegOf); lhs->push(idTemp); lhs->push(temp1);
    // Rhs is just sgnex(size, size2, src)
    src->prep(size2);
    src->prep(size);
    src->prep(idSgnEx);
    RTAssgn* pRT = new RTAssgn(lhs, src, size2);
    temp = new SemStr(*lhs);
    return pRT;
}

// Macro to handle bumping register if reqd
#define ADDBUMP {if (bump) bumpRegister(RTs, bump, bumpr);}

/*==============================================================================
 * FUNCTION:       NJMCDecoder::decodeLowLevelInstruction
 * OVERVIEW:       Decodes a machine instruction and returns an instantiated
 *                 list of RTs.
 * PARAMETERS:     hostPC - the address of the pc in the loaded Elf object
 *                 pc - the virtual address of the pc
 *                 result - a reference parameter that has a field that will be
 *                   set to false if an invalid or UNIMP instruction was decoded
 * RETURNS:        the instantiated list of RTs
 *============================================================================*/
list<RT*>* NJMCDecoder::decodeLowLevelInstruction (ADDRESS hostPC, ADDRESS pc,
    DecodeResult& result) 
{
  //  This uses both patterned and non-patterned constructors in
  //  its matching statement. For the non-patterened constructors,
  //  all decoding variables are set here. For the patterned constructors,
  //  the name of the opcode is copied into the opcodeStr variable and
  //  the decoding variables are set by a "second round" decoding method.

    result.numBytes = 0;
    prevIsTrap = IsTrap;            // Remember if last instr was a trap
    IsTrap = false;
    int delta = hostPC - pc;

    // The list of instantiated RTs.
    list<RT*>* RTs = NULL;

  match hostPC to

    | _toCCR (i8) [name]  => {
    // _toCCR   is  andiToCCR | eoriToCCR | oriToCCR
                        RTs = instantiate(pc, name, DIS_I8);
                    }



    | _immEAb (i8, ea) [name]             => {
    // _toSR (i16) [names] is privileged (we should never see it)
    // _immEAb  is  addib | andib | cmpib | eorib | orib | subib
                        int bump = 0, bumpr;
                        chop2ndLast(name);
                        RTs = instantiate(pc, sslName, DIS_I8, daEA(ea, pc,
                            bump, bumpr, 8));
                        ADDBUMP;
                        result.numBytes += 2;
                    }


    | _immEAb^".ex" (i8, eax, x) [name]   => {
                        chopBoth(name);
                        RTs = instantiate(pc, sslName, DIS_I8,
                            daEAX(eax, x, result, pc, 8));
                        result.numBytes += 2;
                    }
    
    | _immEAw (i16, ea) [name]            => {
                        int bump = 0, bumpr;
                        chop2ndLast(name);
                        RTs = instantiate(pc, sslName, DIS_I16, daEA(ea, pc,
                            bump, bumpr, 16));
                        ADDBUMP;
                        result.numBytes += 2;
                    }


    | _immEAw^".ex" (i16, eax, x) [name]  => {
                        chopBoth(name);
                        RTs = instantiate(pc, sslName, DIS_I16,
                            daEAX(eax, x, result, pc, 16));
                        result.numBytes += 2;
                    }
    
    | _immEAl (i32, ea) [name]            => {
                        int bump = 0, bumpr;
                        chop2ndLast(name);
                        RTs = instantiate(pc, sslName, DIS_I32, daEA(ea, pc,
                            bump, bumpr, 32));
                        ADDBUMP;
                        result.numBytes += 4;
                    }

    | _immEAl^".ex" (i32, eax, x) [name]  => {
                        chopBoth(name);
                        RTs = instantiate(pc, sslName, DIS_I32,
                            daEAX(eax, x, result, pc, 32));
                        result.numBytes += 4;
                    }

    
    | _bits (n, ea) [name] => {
        // _bits    is  bchg  | bclr  | bset
        // _bitsi   is  bchgi | bclri | bseti
        // This series are assumed to be 8 bits where memory is involved,
        // or 32 for registers
                        int bump = 0, bumpr;
                        RTs = instantiate(pc, name, DIS_DN(32), daEA(ea, pc,
                            bump, bumpr, 8));
                        ADDBUMP;
                    }

    | _bits^".ex" (n, eax, x) [name] => {
                        chopDotex(name);
                        RTs = instantiate(pc, sslName, DIS_DN(32),
                            daEAX(eax, x, result, pc, 8));
                    }

    | _bitsi (i8, ea) [name] => {
                        int bump = 0, bumpr;
                        strcpy(sslName, name);
                        sslName[4] = '\0';         // Truncate name
                        RTs = instantiate(pc, sslName, DIS_I8, daEA(ea, pc,
                            bump, bumpr, 8));
                        ADDBUMP;
                        result.numBytes += 2;
                    }

    | _bitsi^".ex" (i8, eax, x) [name]  => {
                        strcpy(sslName, name);
                        sslName[4] = '\0';         // Truncate name
                        RTs = instantiate(pc, sslName, DIS_I8,
                            daEAX(eax, x, result, pc, 8));
                        result.numBytes += 2;
                    }
    
    | btst (n, ea) => {
        // btst, btsti
            int bump = 0, bumpr;
            RTs = instantiate(pc, "btst", DIS_DN(32), dBEA(ea, pc, bump, bumpr, 8));
            ADDBUMP;
        }
   

    | btst^".ex" (n, eax, x) => { 
            RTs = instantiate (pc, "btst", DIS_DN(32),
                dBEAX (eax, x, result, pc, delta, 8));
        }

    | btsti (i8, ea) => { 
            int bump = 0, bumpr;
            RTs = instantiate (pc, "btst", DIS_I8, dBEA (ea, pc, bump, bumpr, 8)); 
            ADDBUMP;
            result.numBytes += 2; 
        }

    | btsti.ex (i8, eax, x) => { 
            RTs = instantiate (pc, "btst", DIS_I8,
                dBEAX (eax, x, result, pc, delta, 8));
            result.numBytes += 2; 
        // MOVEP: privileged (and uncommon as well)
        }

    | _move (ea, ea2) [name] => { 
        // MOVE
        // _move is  moveb | movew | movel
        // check b|w|l
            int bump = 0, bumpr, siz=32;
            SemStr* t1;     // Ptr to SemStr with src
            SemStr* t2;     // Ptr to SemStr with dest
            SemStr* t3;     // Ptr to SemStr with "temp1"
            strcpy(sslName, name);
            sslName[5] = '\0';  // truncate name
            if (name[4] == 'b') siz = 8;
            if (name[4] == 'w') siz = 16;
            t1 = msEA(ea, pc, bump, bumpr, siz);
            RTs = new list<RT*>;    // Empty list of RTs
            ADDBUMP;                // Src may cause a bump
            bump = 0;
            t2 = mdEA(ea2, pc, bump, bumpr, siz);
            ADDBUMP;                // Dest may cause a bump as well
            // Check for An as a dest, and not 32 bits size
            RT* rt;
            bool sgnex = ((siz != 32) && (t2->getFirstIdx() == idRegOf) &&
              (t2->getThirdIdx() >= 8));
            if (sgnex) {
                // Yes, therefore this is a sign extent to 32 bits
                rt = sgnExTemp(t1, siz, 32, t3);
                siz = 32;
                t2->getType().setSize(32);
                sslName[4] = 'l';       // So the second assignment will be long
            }
            if (!sgnex)     // else
                // Just assign the source to temp1
                rt = assignTemp(t1, siz, t3); 
            // We instantiate RTs2 to be dest = temp1
            list<RT*>* RTs2 = (instantiate (pc, sslName, t3, t2));
            // RTs has 0-2 bumps (in the correct order). We must insert
            // before that dest = temp1, and before that rt (temp1 = src etc)
            RTs->insert(RTs->begin(), RTs2->begin(), RTs2->end());
            RTs->insert(RTs->begin(), rt);
            delete RTs2;
        }

    | _move^".ex" (eax, x, ea2) [name] => { 
            int bump = 0, bumpr, siz=32;
            if (name[4] == 'b') siz = 8;
            if (name[4] == 'w') siz = 16;
            strcpy(sslName, name);
            sslName[5] = '\0';  // truncate name
            RTs = instantiate (pc, sslName,
                msEAX (eax, x, result, pc, delta, siz),
                mdEA (ea2, pc, bump, bumpr, siz));
            ADDBUMP;
        }

    | _move^".exl" (eaxl, d32, ea2) [name] => { 
            int bump = 0, bumpr;
            int siz = 32;
            if (name[4] == 'b') siz = 8;
            if (name[4] == 'w') siz = 16;
            strcpy(sslName, name);
            sslName[5] = '\0';  // truncate name
            RTs = instantiate (pc, sslName,
                msEAXL (eaxl, d32, result, pc, siz),
                mdEA (ea2, pc, bump, bumpr, siz)); 
            ADDBUMP;
        }

    | _move^".mx" (ea, eax2, x2) [name] => { 
            int bump = 0, bumpr;
            int siz = 32;
            if (name[4] == 'b') siz = 8;
            if (name[4] == 'w') siz = 16;
            strcpy(sslName, name);
            sslName[5] = '\0';  // truncate name
            RTs = instantiate (pc, sslName,
                msEA (ea, pc, bump, bumpr, siz), 
                mdEAX (eax2, x2, result, pc, siz)); 
            ADDBUMP;
        }

    | _move^".emx" (eax, x, eax2, x2) [name]      => { 
            int siz = 32;
            if (name[4] == 'b') siz = 8;
            if (name[4] == 'w') siz = 16;
            strcpy(sslName, name);
            sslName[5] = '\0';  // truncate name
            RTs = instantiate (pc, sslName,
                msEAX (eax, x, result, pc, delta, siz),
                mdEAX (eax2, x2, result, pc, siz)); 
        }

    | _move^".emxl" (eaxl, d32, eax2, x2) [name]  => { 
            int siz = 32;
            if (name[4] == 'b') siz = 8;
            if (name[4] == 'w') siz = 16;
            strcpy(sslName, name);
            sslName[5] = '\0';  // truncate name
            RTs = instantiate (pc, sslName,
                msEAXL (eaxl, d32, result, pc, siz),
                mdEAX (eax2, x2, result, pc, siz)); 
        }


    | _oneEAdaB (ea) [name]            => { 
        // One operand instructions 
        //  _oneEAdaB is  clrb | negb | negxb | notb | tstb | nbcd | tas
        //  _oneEAdaW is  clrw | negw | negxw | notw | tstw
        //  _oneEAdaL is  clrl | negl | negxl | notl | tstl
            int bump = 0, bumpr;
            chopDotex(name);
            RTs = instantiate (pc, sslName, daEA (ea, pc, bump, bumpr, 8)); 
            ADDBUMP;
        }  

    | _oneEAdaB^".ex" (eax, x) [name]  => { 
            chopDotex(name);
            RTs = instantiate (pc, sslName, daEAX (eax, x, result, pc, 8)); 
        }

    | _oneEAdaW (ea) [name]            => { 
            int bump = 0, bumpr;
            chopDotex(name);
            RTs = instantiate (pc, sslName, daEA (ea, pc, bump, bumpr, 16)); 
            ADDBUMP;
        }

    | _oneEAdaW^".ex" (eax, x) [name]  => { 
            chopDotex(name);
            RTs = instantiate (pc, sslName, daEAX (eax, x, result, pc, 16)); 
        }

    | _oneEAdaL (ea) [name]            => { 
            int bump = 0, bumpr;
            chopDotex(name);
            RTs = instantiate (pc, sslName, daEA (ea, pc, bump, bumpr, 32));
            ADDBUMP;
        }

    | _oneEAdaL^".ex" (eax, x) [name]  => { 
            chopDotex(name);
            RTs = instantiate (pc, sslName,
                daEAX (eax, x, result, pc, 32));
        }

    | _oneEAc (ea) [name]            => { 
        // _oneEAc   is  jsr | jmp | pea    
            strcpy(sslName, name);
            sslName[3] = '\0'; 
            RTs = instantiate (pc, sslName, cEA (ea, pc, 32)); 
        }

    | _oneEAc^".ex" (eax, x) [name]  => { 
            strcpy(sslName, name);
            sslName[3] = '\0';
            RTs = instantiate (pc, sslName,
                cEAX (eax, x, result, pc, delta, 32)); 
        } 

    | unlk (n) [name]       => { 
        // unlk
            RTs = instantiate (pc, name, DIS_AN); 
        }
    
    | _reg2only (n) [name]           => { 
        // _reg2only is  extw | extl | swap
        // extbl is 68020 specific
            chopDotex(name);
            int siz = 16;
            if (sslName[3] == 'l') siz = 32;
            RTs = instantiate (pc, sslName, DIS_DN(siz)); 
        }

    | _noArg () [name]               => { 
        // _noArg is illegal | reset | nop | rte | rts | trapv | rtr
            RTs = instantiate (pc, name); 
        // MOVE to/from SR is priveleged
        } 

    | moveToCCR (ea) [name]         => {
        // MOVE to/from CCR
            int bump = 0, bumpr;
            RTs = instantiate (pc, name, dWEA(ea, pc, bump, bumpr, 16));
            ADDBUMP;
        }

    | moveToCCR.ex (eax, x) [name]  => {
            RTs = instantiate (pc, name,
                dWEAX(eax, x, result, pc, delta, 16));
        }

    | moveFromCCR (ea) [name]       => {
            int bump = 0, bumpr;
            RTs = instantiate (pc, name, daEA(ea, pc, bump, bumpr, 16));
            ADDBUMP;
        }

    | moveFromCCR.ex (eax, x) [name] => {
            RTs = instantiate (pc, name, daEAX(eax, x, result, pc, 16));
        }

    | trap (d4) => {
        // Trap: as far as Palm Pilots are concerned, these are NOPs, except
        // that an A-line instruction can then legally follow
            RTs = instantiate(pc, "NOP");
            IsTrap = true;
            assert(d4 == d4);       // Suppress unused var warning
        }

    | link (n, i16) [name]          => { 
        // link
            RTs = instantiate (pc, name, DIS_AN, DIS_I16); 
            result.numBytes += 2; 
        // moveFromUSP, moveToUSP privileged
                    }
    
    | movermw (i16, ea)             => { 
        // MOVEA -- These are any moves where the destination is an address reg
        // MOVEM rm means registers to memory
        // HACK! This requires work! Need to bump after EACH register has been
        // moved! Mostly, these will be in prologues/epilogues
            int bump = 0, bumpr;
            RTs = instantiate (pc, "storem.w", DIS_I16,
                rmEA (ea, pc, bump, bumpr, 16));
            result.numBytes += 2; 
            ADDBUMP;
        }   

    | movermw.ex (i16, eax, x) => { 
            RTs = instantiate (pc, "storem.w", DIS_I16, 
                rmEAX (eax, x, result, pc, 16)); 
            result.numBytes += 2; 
        }  

    | movemrw (ea, i16)        => {
        // HACK! Requires work
            int bump = 0, bumpr;
            RTs = instantiate (pc, "loadm.w", DIS_I16,
                mrEA (ea, pc, bump, bumpr, 16));
            result.numBytes += 2;
            ADDBUMP;
        }

    | movemrw.ex (eax, x, i16) => {
            RTs = instantiate (pc, "loadm.w", DIS_I16, 
                mrEAX (eax, x, result, pc, delta, 16)); 
            result.numBytes += 2; 
        }  

    | moverml (i16, ea)        => { 
            // HACK! Needs work
            int bump = 0, bumpr;
            RTs = instantiate (pc, "storem.l", DIS_I16,
                rmEA (ea, pc, bump, bumpr, 32));
            result.numBytes += 2; 
            ADDBUMP;
        } 

    | moverml.ex (i16, eax, x) => { 
            RTs = instantiate (pc, "storem.l", DIS_I16, 
                rmEAX(eax, x, result, pc, 32));
            result.numBytes += 2; 
        }   

    | movemrl (ea, i16)        => {
        // HACK! Requires work
            int bump = 0, bumpr;
            RTs = instantiate (pc, "loadm.l", DIS_I16,
                mrEA (ea, pc, bump, bumpr, 32));
            result.numBytes += 2; 
            ADDBUMP;
        } 

    | movemrl.ex (eax, x, i16) => {
            RTs = instantiate (pc, "loadm.l", DIS_I16, 
                    mrEAX(eax, x, result, pc, delta, 32));
            result.numBytes += 2; 
        }   

    | lea (ea, n)            => { 
        // lea    
            RTs = instantiate (pc, "lea", cEA (ea, pc, 32), DIS_AN); 
        }  

    | lea.ex (eax, x, n)     => { 
            RTs = instantiate (pc, "lea",
                cEAX (eax, x, result, pc, delta, 32), DIS_AN); 
        }

    | _aluqB (i8, ea) [name]            => { 
        // ADDQ, SUBQ     
        // The data parameter is 3 bits (d3) but a byte will be returned anyway
            int bump = 0, bumpr;
            chop2ndLast(name);          // addqb -> addb
            if (i8 == 0) i8 = 8;        // Quirk of the addq/subq instr
            RTs = instantiate (pc, sslName, DIS_I8, alEA (ea, pc, bump, bumpr, 8)); 
            ADDBUMP;
        }

    | _aluqB^".ex" (i8, eax, x) [name]  => { 
            chopBoth(name);             // addqb.ex -> addb
            sslName[3] = '\0'; 
            if (i8 == 0) i8 = 8;        // Quirk of the addq/subq instr
            RTs = instantiate (pc, sslName, DIS_I8, alEAX (eax, x, result, pc, 8));
        }

    | _aluqW (i8, ea) [name]            => { 
            int bump = 0, bumpr;
            if (i8 == 0) i8 = 8;        // Quirk of the addq/subq instr
            SemStr* dst = alEA (ea, pc, bump, bumpr, 16);
            bool b = (dst->getFirstIdx() == idRegOf) && 
                (dst->getSecondIdx() == idIntConst) && 
                (dst->getThirdIdx() >= 8);
            if (b) {
                // We have addq/subq to an address register. These do not
                // affect the flags (all others do). Also, the instruction
                // is always 32 bits. So we give it a different SSL name
                strcpy(sslName, name);
                sslName[3] = '\0';
                strcat(sslName, "qa");     // addqw -> addqa
            }
            //if (!b)                       // Can't use else
            } else {                        // Can use else if not at start
                chop2ndLast(name);          // addqw -> addw
            }
            RTs = instantiate (pc, sslName, DIS_I8, dst); 
            ADDBUMP;
        } 

    | _aluqW^".ex" (i8, eax, x) [name]  => { 
            chopBoth(name);             // addqb.ex -> addb
            if (i8 == 0) i8 = 8;        // Quirk of the addq/subq instr
            RTs = instantiate (pc, sslName, DIS_I8,
                alEAX (eax, x, result, pc, 16)); 
        } 

    | _aluqL (i8, ea) [name]            => { 
            int bump = 0, bumpr;
            if (i8 == 0) i8 = 8;        // Quirk of the addq/subq instr
            SemStr* dst = alEA (ea, pc, bump, bumpr, 32);
            bool b = (dst->getFirstIdx() == idRegOf) && 
                (dst->getSecondIdx() == idIntConst) && 
                (dst->getThirdIdx() >= 8);
            if (b) {
                // We have addq/subq to an address register. These do not
                // affect the flags (all others do). So we give it a different
                // SSL name
                strcpy(sslName, name);
                sslName[3] = '\0';
                strcat(sslName, "qa");      // subl -> subqa
            }
            //if (!b)                         // Can't use else
            } else {
                chop2ndLast(name);          // addqw -> addw
            }
            RTs = instantiate (pc, sslName, DIS_I8, dst); 
            ADDBUMP;
        }   

    | _aluqL^".ex" (i8, eax, x) [name]  => { 
            chopBoth(name);             // addql.ex -> addl
            if (i8 == 0) i8 = 8;        // Quirk of the addq/subq instr
            RTs = instantiate (pc, sslName, DIS_I8, alEAX (eax, x, result, pc, 32));
        }   

    | _dbcc (n, i16) [name]  => { 
        // DBcc     
            RTs = instantiate (pc, name, DIS_DN(32), DIS_I16); 
            result.numBytes += 2; 
        }
    
    | _s (ea) [name]           => { 
        // Scc
        // I assume that these are 8 bits where memory is involved, but
        // 32 where registers are involved
            int bump = 0, bumpr;
            RTs = instantiate (pc, name, daEA (ea, pc, bump, bumpr, 8));
            ADDBUMP;
        }

    | _s^".ex" (eax, x) [name] => { 
            RTs = instantiate (pc, name, daEAX (eax, x, result, pc, 8));
        } 

    
    | _uBranch (a) [name]  => { 
        // _uBranch is  bra | bsr
            strcpy(sslName, name);
            if (strcmp (sslName, "bsr") == 0)
                strcpy (sslName, "jsr"); 
            RTs = instantiate (pc, sslName, BTA (a, result, pc)); 
        }

    | _br(a) [name]  => { 
        // Bcc
            RTs = instantiate (pc, name, BTA (a, result, pc)); 
        }

    | moveq (i8, n)  => { 
        // moveq (semantics of move immediate long)
            RTs = instantiate (pc, "movel", DIS_I8, DIS_DN(32)); 
        } 

    | _alurdw (ea, n) [name]            => { 
        // _alurdw is divs | divu | muls | mulu
        //// in order for this to match, the 'w' needs to be dropped off the
        //// SSL names DIVUw and MUL[idx]w
            int bump = 0, bumpr;
            RTs = instantiate (pc, name, dWEA (ea, pc, bump, bumpr, 16), DIS_DN(16)); 
            ADDBUMP;
        }   

    | _alurdw^".ex" (eax, x, n) [name]  => { 
            RTs = instantiate (pc, name,
                dWEAX (eax, x, result, pc, delta, 16), DIS_DN(16)); 
        }

    | _twoRegRB (n, n2) [name]  => {
        // _twoReg** (addx | subx | abcd | sbcd | cmp) 
            strcpy(sslName, name);
            if (strncmp(sslName, "cmp", 3) == 0) sslName[3] = '\0';
            sslName[4] = '\0';
            strcat(sslName, "b");         // Force 8 bit size
            RTs = instantiate (pc, sslName, DIS_DN(8), DIS_DN2(8));
        } 

    | _twoRegMB (n, n2) [name]  => {
            int bump = 0, bumpr;
            strcpy(sslName, name);
            if (strncmp(sslName, "cmp", 3) == 0) sslName[3] = '\0';
            sslName[4] = '\0';
            strcat(sslName, "b");         // Force 8 bit size
            SemStr* t1;
            RT* rt = assignTemp(pPreDec(n, bump, bumpr, 8), 8, t1);
            ADDBUMP;
            bump = 0;
            RTs = instantiate (pc, sslName, t1, pPreDec(n2, bump, bumpr, 8));
            RTs->insert(RTs->begin(), rt);
            ADDBUMP;
        } 

    | _twoRegRW (n, n2) [name]  => {
            strcpy(sslName, name);
            if (strncmp(sslName, "cmp", 3) == 0) sslName[3] = '\0';
            sslName[4] = '\0';
            strcat(name, "w");         // Force 16 bit size
            RTs = instantiate (pc, sslName, DIS_DN(16), DIS_DN2(16));
        } 

    | _twoRegMW (n, n2) [name]  => {
            int bump = 0, bumpr;
            strcpy(sslName, name);
            if (strncmp(sslName, "cmp", 3) == 0) sslName[3] = '\0';
            sslName[4] = '\0';
            strcat(sslName, "w");         // Force 16 bit size
            SemStr* t1;
            RT* rt = assignTemp(pPreDec(n, bump, bumpr, 16), 16, t1);
            ADDBUMP;
            bump = 0;
            RTs = instantiate (pc, sslName, t1, pPreDec(n2, bump, bumpr, 16));
            RTs->insert(RTs->begin(), rt);
            ADDBUMP;
        } 

    | _twoRegRL (n, n2) [name]  => {
            strcpy(sslName, name);
            if (strncmp(sslName, "cmp", 3) == 0) sslName[3] = '\0';
            sslName[4] = '\0';
            strcat(sslName, "l");         // Force 32 bit size
            RTs = instantiate (pc, sslName, DIS_DN(32), DIS_DN2(32));
        } 

    | _twoRegML (n, n2) [name]  => {
            int bump = 0, bumpr;
            strcpy(sslName, name);
            if (strncmp(sslName, "cmp", 3) == 0) sslName[3] = '\0';
            sslName[4] = '\0';
            strcat(sslName, "l");         // Force 32 bit size
            SemStr* t1;
            RT* rt = assignTemp(pPreDec(n, bump, bumpr, 32), 32, t1);
            ADDBUMP;
            bump = 0;
            RTs = instantiate (pc, sslName, t1, pPreDec(n2, bump, bumpr, 32));
            RTs->insert(RTs->begin(), rt);
            ADDBUMP;
        } 

    | _alurdB (ea, n) [name]            => {
        // _alurdB  is  andrb | orrb
            int bump = 0, bumpr;
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, dEA(ea, pc, bump, bumpr, 8), DIS_DN(8));
            ADDBUMP;
        }

    | _alurdB^".ex" (eax, x, n) [name]  => {
            chopBoth(name);
            RTs = instantiate (pc, sslName,
                dEAX(eax, x, result, pc, delta, 8), DIS_DN(8));
        }

    | _alurdW (ea, n) [name]            => {
            int bump = 0, bumpr;
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, dEA(ea, pc, bump, bumpr, 16),
                DIS_DN(16));
            ADDBUMP;
        }

    | _alurdW^".ex" (eax, x, n) [name]  => {
            chopBoth(name);
            RTs = instantiate (pc, sslName,
                dEAX(eax, x, result, pc, delta, 16), DIS_DN(16));
        }

    | _alurdL (ea, n) [name]            => {
            int bump = 0, bumpr;
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, dEA(ea, pc, bump, bumpr, 32), DIS_DN(32));
            ADDBUMP;
        }

    | _alurdL^".ex" (eax, x, n) [name]  => {
            chopBoth(name);
            RTs = instantiate (pc, sslName,
                dEAX(eax, x, result, pc, delta, 32), DIS_DN(32));
        }


    | _alumB (n, ea) [name]             => {
        // _alumB   is  addmb | andmb | ormb  | submb
            int bump = 0, bumpr;
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_DN(8), maEA(ea, pc, bump, bumpr, 8));
            ADDBUMP;
        }

    | _alumB^".ex" (n, eax, x) [name]   => {
            chopBoth(name);
            RTs = instantiate (pc, sslName, DIS_DN(8),
                maEAX(eax, x, result, pc, 8));
        }

    | _alumW (n, ea) [name]             => {
            int bump = 0, bumpr;
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_DN(16), maEA(ea, pc, bump, bumpr,
                16));
            ADDBUMP;
        }

    | _alumW^".ex" (n, eax, x) [name]   => {
            chopBoth(name);
            RTs = instantiate (pc, sslName, DIS_DN(16),
                maEAX(eax, x, result, pc, 16));
        }

    | _alumL (n, ea) [name]             => {
            int bump = 0, bumpr;
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_DN(32), maEA(ea, pc, bump, bumpr,
                32));
            ADDBUMP;
        }

    | _alumL^".ex" (n, eax, x) [name]   => {
            chopBoth(name);
            RTs = instantiate (pc, sslName, DIS_DN(32),
                maEAX(eax, x, result, pc, 32));
        }



    | _aluaW (ea, n) [name]             => {
        // _aluaW   is  addaw | cmpaw | subaw 
            int bump = 0, bumpr;
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, awlEA(ea, pc, bump, bumpr, 16), DIS_AN);
            ADDBUMP;
        }

    | _aluaW^".ex" (eax, x, n) [name]   => {
            chopBoth(name);
            RTs = instantiate(pc, sslName,
                awlEAX(eax, x, result, pc, delta, 16), DIS_AN);
        }

    | _aluaL (ea, n) [name]             => {
            int bump = 0, bumpr;
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, awlEA(ea, pc, bump, bumpr, 32), DIS_AN);
            ADDBUMP;
        }

    | _aluaL^".ex" (eax, x, n) [name]   => {
            chopBoth(name);
            RTs = instantiate(pc, sslName,
                awlEAX(eax, x, result, pc, delta, 32), DIS_AN);
        }

    | eorb (n, ea) [name] => {
            int bump = 0, bumpr;
            RTs = instantiate (pc, name, DIS_DN(8), daEA(ea, pc, bump, bumpr, 8));
            ADDBUMP;
        }

    | eorb.ex (n, eax, x) [name] => {
            RTs = instantiate (pc, name, DIS_DN(8),
                daEAX(eax, x, result, pc, 8));
        }

    | eorw (n, ea) [name] => {
            int bump = 0, bumpr;
            RTs = instantiate (pc, name, DIS_DN(16), daEA(ea, pc, bump, bumpr, 16));
            ADDBUMP;
        }

    | eorw.ex (n, eax, x) [name] => {
            RTs = instantiate (pc, name, DIS_DN(16),
                daEAX(eax, x, result, pc, 16));
        }

    | eorl (n, ea) [name] => {
            int bump = 0, bumpr;
            RTs = instantiate (pc, name, DIS_DN(32), daEA(ea, pc, bump, bumpr, 32));
            ADDBUMP;
        }

    | eorl.ex (n, eax, x) [name] => {
            RTs = instantiate (pc, name, DIS_DN(32),
                daEAX(eax, x, result, pc, 32));
        }

    
    | exgdd (n, n2) [name] => {
            RTs = instantiate (pc, name, DIS_DN(32), DIS_DN2(32));
        }

    | exgaa (n, n2) [name] => {
            RTs = instantiate (pc, name, DIS_AN, DIS_AN2);
        }

    | exgda (n, n2) [name] => {
            RTs = instantiate (pc, name, DIS_DN(32), DIS_AN2);
        }

    
    | _alurB (ea, n) [name]  => {
        // ADD, AND, CHK, CMP, CMPA, DIVS, DIVU, MULS, MULU, OR, SUB, SUBA
            int bump = 0, bumpr;
            chopDotex(name);           // Fix the name
            RTs = instantiate (pc, sslName, amEA(ea, pc, bump, bumpr, 8), DIS_DN(8));
            ADDBUMP;
        }

    | _alurB^".ex" (eax, x, n) [name]   => {
            chopDotex(name);           // Fix the name
            RTs = instantiate (pc, sslName,
                amEAX(eax, x, result, pc, delta, 8), DIS_DN(8));
        }
    
    | _alurW (ea, n) [name]  => {
            int bump = 0, bumpr;
            chopDotex(name);           // Fix the name
            RTs = instantiate (pc, sslName, amEA(ea, pc, bump, bumpr, 16),
                DIS_DN(16));
            ADDBUMP;
        }

    | _alurW^".ex" (eax, x, n) [name]   => {
            chopDotex(name);           // Fix the name
            RTs = instantiate (pc, sslName,
                amEAX(eax, x, result, pc, delta, 16), DIS_DN(16));
        }

    | _alurL (ea, n) [name]  => {
            int bump = 0, bumpr;
            chopDotex(name);           // Fix the name
            RTs = instantiate (pc, sslName, amEA(ea, pc, bump, bumpr, 32),
                DIS_DN(32));
            ADDBUMP;
        }

    | _alurL^".ex" (eax, x, n) [name]  => {
            chopDotex(name);           // Fix the name
            RTs = instantiate (pc, sslName,
                amEAX(eax, x, result, pc, delta, 32), DIS_DN(32));
        }


    | _shiftMR (ea) [name]  => {
        // ASL, ASR, LSL, LSR, ROL, ROR, ROXL, ROXR
            int bump = 0, bumpr;
            chopDotex(name);           // Fix the name
            int lst = strlen(sslName) - 1;
            int siz = 32;
            if (sslName[lst] == 'b') siz = 8;
            if (sslName[lst] == 'w') siz = 16;
            RTs = instantiate (pc, sslName, maEA(ea, pc, bump, bumpr, siz));
            ADDBUMP;
        }

    | _shiftMR^".ex" (eax, x) [name]  => {
            chopDotex(name);           // Fix the name
            RTs = instantiate (pc, sslName, maEAX(eax, x, result, pc, 16));
        }

    | _shiftML (ea) [name]            => {
            int bump = 0, bumpr;
            chopDotex(name);           // Fix the name
            int lst = strlen(sslName) - 1;
            int siz = 32;
            if (sslName[lst] == 'b') siz = 8;
            if (sslName[lst] == 'w') siz = 16;
            RTs = instantiate (pc, sslName, maEA(ea, pc, bump, bumpr, siz));
            ADDBUMP;
        }

    | _shiftML^".ex" (eax, x) [name]  => {
            chopDotex(name);           // Fix the name
            RTs = instantiate (pc, sslName, maEAX(eax, x, result, pc, 16));
        }

    
    | _shiftIRB (i8, n) [name]   => {
        // _shiftIRB    is  asrib | lsrib | rorib | roxrib
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(8));
        }
    
    | _shiftILB (i8, n) [name]   => {
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(8));
        }
    
    | _shiftIRW (i8, n) [name]   => {
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(16));
        }
    
    | _shiftILW (i8, n) [name]   => {
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(16));
        }
    
    | _shiftIRL (i8, n) [name]   => {
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(32));
        }
    
    | _shiftILL (i8, n) [name]   => {
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(32));
        }
    
    
    | _shiftRRB (n, n2) [name]   => {
        // _shiftRRB    is  asrrb | lsrrb | rorrb | roxrrb
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_DN(8), DIS_DN2(8));
        }

    | _shiftRLB (n, n2) [name]   => {
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_DN(8), DIS_DN2(8));
        }
    
    | _shiftRRW (n, n2) [name]   => {
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_DN(16), DIS_DN2(16));
        }
    
    | _shiftRLW (n, n2) [name]   => {
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_DN(16), DIS_DN2(16));
        }
    
    | _shiftRRL (n, n2) [name]   => {
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_DN(32), DIS_DN2(32));
        }
    
    | _shiftRLL (n, n2) [name]   => {
            chop2ndLast(name);
            RTs = instantiate (pc, sslName, DIS_DN(32), DIS_DN2(32));
        }

    else    {   // the toolkit reserves "else" as a keyword, hence this code
                if (!prevIsTrap) {
                    ostrstream ost;
                    ost << "Undecoded instruction " << hex << getWord(pc+delta);
                    ost << " at " << pc;
                    warning(str(ost));
                    RTs = NULL;
                }
                if (prevIsTrap) 
                    decodeTrapName (pc);
    }  
  endmatch

    result.numBytes += 2;           // Count the main opcode
    return RTs;
}


