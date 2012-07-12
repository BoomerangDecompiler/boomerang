#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>

#line 2 "machine/mc68k/decoder_low.m"
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
SWord getWord (ADDRESS lc)
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



#line 217 "machine/mc68k/decoder_low.m"
{ 
  dword MATCH_p = 
    
    #line 217 "machine/mc68k/decoder_low.m"
    hostPC
    ;
  char *MATCH_name;
  static char *MATCH_name_sz_1[] = {"orrb", "oriw", "oril", "bset", };
  static char *MATCH_name_sz_3[] = {"orrb.ex", "andiw", "andil", "bset.ex", };
  static char *MATCH_name_sz_5[] = {"andrb", "subiw", "subil", "divu", };
  static char *MATCH_name_sz_7[] = {
    "andrb.ex", "addiw", "addil", "divu.ex", 
  };
  static char *MATCH_name_sz_12[] = {(char *)0, "eoriw", "eoril", "mulu", };
  static char *MATCH_name_sz_14[] = {
    (char *)0, "cmpiw", "cmpil", "mulu.ex", 
  };
  static char *MATCH_name_sz_15[] = {(char *)0, "bchg", "bclr", };
  static char *MATCH_name_sz_17[] = {(char *)0, "oriw.ex", "oril.ex", };
  static char *MATCH_name_sz_19[] = {(char *)0, "andiw.ex", "andil.ex", };
  static char *MATCH_name_sz_21[] = {(char *)0, "subiw.ex", "subil.ex", };
  static char *MATCH_name_sz_23[] = {(char *)0, "addiw.ex", "addil.ex", };
  static char *MATCH_name_sz_28[] = {(char *)0, "eoriw.ex", "eoril.ex", };
  static char *MATCH_name_sz_30[] = {(char *)0, "cmpiw.ex", "cmpil.ex", };
  static char *MATCH_name_sz_31[] = {(char *)0, "bchg.ex", "bclr.ex", };
  static char *MATCH_name_adrm_35[] = {
    "moveb", "moveb", "moveb", "moveb", "moveb", "moveb.ex", "moveb.ex", 
    "rorrb", 
  };
  static char *MATCH_name_reg2_36[] = {
    "moveb.ex", "moveb.exl", "moveb.ex", "moveb.ex", "moveb.ex", "rts", 
    "trapv", "rtr", 
  };
  static char *MATCH_name_adrm_37[] = {
    "moveb.mx", "moveb.mx", "moveb.mx", "moveb.mx", "moveb.mx", "moveb.emx", 
    "moveb.emx", "rorrw", 
  };
  static char *MATCH_name_reg2_38[] = {
    "moveb.emx", "moveb.emxl", "moveb.emx", "moveb.emx", "moveb.emx", 
  };
  static char *MATCH_name_MDadrm_39[] = {
    "moveb", "moveb", "moveb", "moveb", "moveb", "moveb.mx", "moveb.mx", 
    "moveb.mx", 
  };
  static char *MATCH_name_MDadrm_40[] = {
    "moveb.ex", "moveb.ex", "moveb.ex", "moveb.ex", "moveb.ex", "moveb.emx", 
    "moveb.emx", "moveb.emx", 
  };
  static char *MATCH_name_MDadrm_41[] = {
    "moveb.exl", "moveb.exl", "moveb.exl", "moveb.exl", "moveb.exl", 
    "moveb.emxl", "moveb.emxl", "moveb.emxl", 
  };
  static char *MATCH_name_adrm_42[] = {
    "movel", "movel", "movel", "movel", "movel", "movel.ex", "movel.ex", 
    "rorrl", 
  };
  static char *MATCH_name_reg2_43[] = {
    "movel.ex", "movel.exl", "movel.ex", "movel.ex", "movel.exl", 
  };
  static char *MATCH_name_adrm_44[] = {
    "movel.mx", "movel.mx", "movel.mx", "movel.mx", "movel.mx", "movel.emx", 
    "movel.emx", "rolrb", 
  };
  static char *MATCH_name_reg2_45[] = {
    "movel.emx", "movel.emxl", "movel.emx", "movel.emx", "movel.emxl", 
  };
  static char *MATCH_name_MDadrm_46[] = {
    "movel", "movel", "movel", "movel", "movel", "movel.mx", "movel.mx", 
    "movel.mx", 
  };
  static char *MATCH_name_MDadrm_47[] = {
    "movel.ex", "movel.ex", "movel.ex", "movel.ex", "movel.ex", "movel.emx", 
    "movel.emx", "movel.emx", 
  };
  static char *MATCH_name_MDadrm_48[] = {
    "movel.exl", "movel.exl", "movel.exl", "movel.exl", "movel.exl", 
    "movel.emxl", "movel.emxl", "movel.emxl", 
  };
  static char *MATCH_name_adrm_49[] = {
    "movew", "movew", "movew", "movew", "movew", "movew.ex", "movew.ex", 
    "rolrw", 
  };
  static char *MATCH_name_reg2_50[] = {
    "movew.ex", "movew.exl", "movew.ex", "movew.ex", "movew.ex", 
  };
  static char *MATCH_name_adrm_51[] = {
    "movew.mx", "movew.mx", "movew.mx", "movew.mx", "movew.mx", "movew.emx", 
    "movew.emx", "rolrl", 
  };
  static char *MATCH_name_reg2_52[] = {
    "movew.emx", "movew.emxl", "movew.emx", "movew.emx", "movew.emx", 
  };
  static char *MATCH_name_MDadrm_53[] = {
    "movew", "movew", "movew", "movew", "movew", "movew.mx", "movew.mx", 
    "movew.mx", 
  };
  static char *MATCH_name_MDadrm_54[] = {
    "movew.ex", "movew.ex", "movew.ex", "movew.ex", "movew.ex", "movew.emx", 
    "movew.emx", "movew.emx", 
  };
  static char *MATCH_name_MDadrm_55[] = {
    "movew.exl", "movew.exl", "movew.exl", "movew.exl", "movew.exl", 
    "movew.emxl", "movew.emxl", "movew.emxl", 
  };
  static char *MATCH_name_reg1_56[] = {
    "negxb", "clrb", "negb", "notb", "nbcd", "tstb", (char *)0, "jsr", 
  };
  static char *MATCH_name_reg1_57[] = {
    "negxb.ex", "clrb.ex", "negb.ex", "notb.ex", "nbcd.ex", "tstb.ex", 
    (char *)0, "jsr.ex", 
  };
  static char *MATCH_name_adrm_58[] = {
    "negxw", "addqb", "negxw", "negxw", "negxw", "negxw.ex", "negxw.ex", 
  };
  static char *MATCH_name_reg2_59[] = {
    "negxw.ex", "negxw.ex", "pea.ex", "pea.ex", "moveToCCR.ex", 
  };
  static char *MATCH_name_adrm_60[] = {
    "clrw", "subqb", "clrw", "clrw", "clrw", "clrw.ex", "clrw.ex", 
  };
  static char *MATCH_name_reg2_61[] = {
    "clrw.ex", "clrw.ex", "moveToCCR.ex", "rte", "illegal", 
  };
  static char *MATCH_name_adrm_62[] = {
    "negw", "addqw", "negw", "negw", "negw", "negw.ex", "negw.ex", 
  };
  static char *MATCH_name_reg2_63[] = {
    "negw.ex", "negw.ex", "jmp.ex", "moveToCCR.ex", "chk.ex", 
  };
  static char *MATCH_name_adrm_64[] = {
    "notw", "subqw", "notw", "notw", "notw", "notw.ex", "notw.ex", 
  };
  static char *MATCH_name_reg2_65[] = {
    "notw.ex", "notw.ex", "chk.ex", "jmp.ex", "divs.ex", 
  };
  static char *MATCH_name_adrm_66[] = {
    "swap", "addql", "pea", "tstw", "tstw", "pea.ex", "pea.ex", 
  };
  static char *MATCH_name_reg2_67[] = {
    "pea.ex", "pea.ex", "divs.ex", "chk.ex", "subrb.ex", 
  };
  static char *MATCH_name_adrm_68[] = {
    "tstw", "subql", "tstw", "unlk", "moveFromCCR", "tstw.ex", "tstw.ex", 
  };
  static char *MATCH_name_reg2_69[] = {
    "tstw.ex", "tstw.ex", "subrb.ex", "divs.ex", "subrw.ex", 
  };
  static char *MATCH_name_reg2_71[] = {
    "reset", "nop", "subrw.ex", "subrb.ex", "subrl.ex", 
  };
  static char *MATCH_name_reg1_72[] = {
    "negxl", "clrl", "negl", "notl", "extw", "tstl", 
  };
  static char *MATCH_name_reg1_73[] = {
    "negxl.ex", "clrl.ex", "negl.ex", "notl.ex", (char *)0, "tstl.ex", 
  };
  static char *MATCH_name_adrm_74[] = {
    "moveFromCCR", "sbcdm", "moveFromCCR", "moveFromCCR", "moveToCCR", 
    "moveFromCCR.ex", "moveFromCCR.ex", 
  };
  static char *MATCH_name_reg2_75[] = {
    "moveFromCCR.ex", "moveFromCCR.ex", "subrl.ex", "subrw.ex", "subaw.ex", 
  };
  static char *MATCH_name_adrm_76[] = {
    "moveToCCR", "subrb", "moveToCCR", "moveToCCR", "tas", "moveToCCR.ex", 
    "moveToCCR.ex", 
  };
  static char *MATCH_name_reg2_77[] = {
    "moveToCCR.ex", "moveToCCR.ex", "subaw.ex", "subrl.ex", "subal.ex", 
  };
  static char *MATCH_name_adrm_78[] = {
    "extl", "subrw", "tas", "tas", "chk", "tas.ex", "tas.ex", 
  };
  static char *MATCH_name_adrm_79[] = {
    "tas", "subrl", "jmp", "chk", "addqb", "jmp.ex", "jmp.ex", 
  };
  static char *MATCH_name_reg2_80[] = {
    "tas.ex", "tas.ex", "subal.ex", "subaw.ex", "cmpb.ex", 
  };
  static char *MATCH_name_reg2_81[] = {
    "jmp.ex", "jmp.ex", "cmpb.ex", "subal.ex", "cmpw.ex", 
  };
  static char *MATCH_name_adrm_82[] = {
    "chk", "subaw", "chk", "addqb", "subqb", "chk.ex", "chk.ex", 
  };
  static char *MATCH_name_reg2_83[] = {
    "chk.ex", "chk.ex", "cmpw.ex", "cmpb.ex", "cmpl.ex", 
  };
  static char *MATCH_name_adrm_84[] = {
    "addqb", "subxmb", "addqb", "subqb", "addqw", "addqb.ex", "addqb.ex", 
  };
  static char *MATCH_name_reg2_85[] = {
    "addqb.ex", "addqb.ex", "cmpl.ex", "cmpw.ex", "cmpaw.ex", 
  };
  static char *MATCH_name_adrm_86[] = {
    "subqb", "subxmw", "subqb", "addqw", "subqw", "subqb.ex", "subqb.ex", 
  };
  static char *MATCH_name_reg2_87[] = {
    "subqb.ex", "subqb.ex", "cmpaw.ex", "cmpl.ex", "cmpal.ex", 
  };
  static char *MATCH_name_adrm_88[] = {
    "addqw", "subxml", "addqw", "subqw", "addql", "addqw.ex", "addqw.ex", 
  };
  static char *MATCH_name_reg2_89[] = {
    "addqw.ex", "addqw.ex", "cmpal.ex", "cmpaw.ex", "muls.ex", 
  };
  static char *MATCH_name_adrm_90[] = {
    "subqw", "subal", "subqw", "addql", "subql", "subqw.ex", "subqw.ex", 
  };
  static char *MATCH_name_reg2_91[] = {
    "subqw.ex", "subqw.ex", "muls.ex", "cmpal.ex", "addrb.ex", 
  };
  static char *MATCH_name_adrm_92[] = {
    "addql", "cmpb", "addql", "subql", "ormb", "addql.ex", "addql.ex", 
  };
  static char *MATCH_name_reg2_93[] = {
    "addql.ex", "addql.ex", "addrb.ex", "muls.ex", "addrw.ex", 
  };
  static char *MATCH_name_adrm_94[] = {
    "subql", "cmpw", "subql", "ormb", "ormw", "subql.ex", "subql.ex", 
  };
  static char *MATCH_name_reg2_95[] = {
    "subql.ex", "subql.ex", "addrw.ex", "addrb.ex", "addrl.ex", 
  };
  static char *MATCH_name_cond_96[] = {
    "st", "sf", "shi", "sls", "scc", "scs", "sne", "seq", "svc", "svs", 
    "spl", "smi", "sge", "slt", "sgt", "sle", 
  };
  static char *MATCH_name_cond_97[] = {
    "dbt", "dbf", "dbhi", "dbls", "dbcc", "dbcs", "dbne", "dbeq", "dbvc", 
    "dbvs", "dbpl", "dbmi", "dbge", "dblt", "dbgt", "dble", 
  };
  static char *MATCH_name_cond_98[] = {
    "st.ex", "sf.ex", "shi.ex", "sls.ex", "scc.ex", "scs.ex", "sne.ex", 
    "seq.ex", "svc.ex", "svs.ex", "spl.ex", "smi.ex", "sge.ex", "slt.ex", 
    "sgt.ex", "sle.ex", 
  };
  static char *MATCH_name_cond_99[] = {
    "bra", "bsr", "bhi", "bls", "bcc", "bcs", "bne", "beq", "bvc", "bvs", 
    "bpl", "bmi", "bge", "blt", "bgt", "ble", 
  };
  static char *MATCH_name_sz_100[] = {(char *)0, "orrw", "orrl", };
  static char *MATCH_name_sz_101[] = {(char *)0, "orrw.ex", "orrl.ex", };
  static char *MATCH_name_adrm_102[] = {
    "sbcdr", "cmpl", "ormb", "ormw", "orml", "ormb.ex", "ormb.ex", 
  };
  static char *MATCH_name_reg2_103[] = {
    "ormb.ex", "ormb.ex", "addrl.ex", "addrw.ex", "addaw.ex", 
  };
  static char *MATCH_name_adrm_104[] = {
    "divs", "cmpaw", "ormw", "orml", "divs", "ormw.ex", "ormw.ex", 
  };
  static char *MATCH_name_reg2_105[] = {
    "ormw.ex", "ormw.ex", "addaw.ex", "addrl.ex", "addal.ex", 
  };
  static char *MATCH_name_adrm_106[] = {
    "subrb", "cmpmb", "orml", "divs", "subrb", "orml.ex", "orml.ex", 
  };
  static char *MATCH_name_reg2_107[] = {
    "orml.ex", "orml.ex", "addal.ex", "addaw.ex", 
  };
  static char *MATCH_name_adrm_108[] = {
    "subrw", "cmpmw", "divs", "subrb", "subrw", "divs.ex", "divs.ex", 
  };
  static char *MATCH_name_reg2_109[] = {
    "divs.ex", "divs.ex", (char *)0, "addal.ex", 
  };
  static char *MATCH_name_adrm_110[] = {
    "subrl", "cmpml", "subrb", "subrw", "subrl", "subrb.ex", "subrb.ex", 
  };
  static char *MATCH_name_adrm_112[] = {
    "subaw", "cmpal", "subrw", "subrl", "subaw", "subrw.ex", "subrw.ex", 
  };
  static char *MATCH_name_adrm_114[] = {
    "subxrb", "abcdm", "subrl", "subaw", "submb", "subrl.ex", "subrl.ex", 
  };
  static char *MATCH_name_adrm_116[] = {
    "subxrw", "exgaa", "subaw", "submb", "submw", "subaw.ex", "subaw.ex", 
  };
  static char *MATCH_name_adrm_118[] = {
    "subxrl", "exgda", "submb", "submw", "subml", "submb.ex", "submb.ex", 
  };
  static char *MATCH_name_adrm_120[] = {
    "subal", "addrb", "submw", "subml", "subal", "submw.ex", "submw.ex", 
  };
  static char *MATCH_name_adrm_122[] = {
    "cmpb", "addrw", "subml", "subal", "cmpb", "subml.ex", "subml.ex", 
  };
  static char *MATCH_name_adrm_124[] = {
    "cmpw", "addrl", "subal", "cmpb", "cmpw", "subal.ex", "subal.ex", 
  };
  static char *MATCH_name_adrm_126[] = {
    "cmpl", "addaw", "cmpb", "cmpw", "cmpl", "cmpb.ex", "cmpb.ex", 
  };
  static char *MATCH_name_adrm_128[] = {
    "cmpaw", "addxmb", "cmpw", "cmpl", "cmpaw", "cmpw.ex", "cmpw.ex", 
  };
  static char *MATCH_name_adrm_130[] = {
    "eorb", "addxmw", "cmpl", "cmpaw", "eorb", "cmpl.ex", "cmpl.ex", 
  };
  static char *MATCH_name_adrm_132[] = {
    "eorw", "addxml", "cmpaw", "eorb", "eorw", "cmpaw.ex", "cmpaw.ex", 
  };
  static char *MATCH_name_adrm_134[] = {
    "eorl", "addal", "eorb", "eorw", "eorl", "eorb.ex", "eorb.ex", 
  };
  static char *MATCH_name_adrm_136[] = {
    "cmpal", "lsrib", "eorw", "eorl", "cmpal", "eorw.ex", "eorw.ex", 
  };
  static char *MATCH_name_adrm_138[] = {
    "abcdr", "lsriw", "eorl", "cmpal", "andmb", "eorl.ex", "eorl.ex", 
  };
  static char *MATCH_name_adrm_140[] = {
    "exgdd", "lsril", "cmpal", "andmb", "andmw", "cmpal.ex", "cmpal.ex", 
  };
  static char *MATCH_name_sz_142[] = {(char *)0, "andrw", "andrl", };
  static char *MATCH_name_sz_143[] = {(char *)0, "andrw.ex", "andrl.ex", };
  static char *MATCH_name_adrm_144[] = {
    "muls", "lslib", "andmb", "andmw", "andml", "andmb.ex", "andmb.ex", 
  };
  static char *MATCH_name_adrm_146[] = {
    "addrb", "lsliw", "andmw", "andml", "muls", "andmw.ex", "andmw.ex", 
  };
  static char *MATCH_name_adrm_148[] = {
    "addrw", "lslil", "andml", "muls", "addrb", "andml.ex", "andml.ex", 
  };
  static char *MATCH_name_adrm_150[] = {
    "addrl", (char *)0, "muls", "addrb", "addrw", "muls.ex", "muls.ex", 
  };
  static char *MATCH_name_adrm_152[] = {
    "addaw", (char *)0, "addrb", "addrw", "addrl", "addrb.ex", "addrb.ex", 
  };
  static char *MATCH_name_adrm_154[] = {
    "addxrb", (char *)0, "addrw", "addrl", "addaw", "addrw.ex", "addrw.ex", 
  };
  static char *MATCH_name_adrm_156[] = {
    "addxrw", (char *)0, "addrl", "addaw", "addmb", "addrl.ex", "addrl.ex", 
  };
  static char *MATCH_name_adrm_158[] = {
    "addxrl", (char *)0, "addaw", "addmb", "addmw", "addaw.ex", "addaw.ex", 
  };
  static char *MATCH_name_adrm_160[] = {
    "addal", (char *)0, "addmb", "addmw", "addml", "addmb.ex", "addmb.ex", 
  };
  static char *MATCH_name_adrm_162[] = {
    "asrib", (char *)0, "addmw", "addml", "addal", "addmw.ex", "addmw.ex", 
  };
  static char *MATCH_name_adrm_164[] = {
    "asriw", (char *)0, "addml", "addal", "asrrb", "addml.ex", "addml.ex", 
  };
  static char *MATCH_name_adrm_166[] = {
    "asril", (char *)0, "addal", "rorib", "asrrw", "addal.ex", "addal.ex", 
  };
  static char *MATCH_name_adrm_168[] = {
    "aslib", (char *)0, "roxrib", "roriw", "asrrl", "lsrrb", "roxrrb", 
  };
  static char *MATCH_name_adrm_169[] = {
    "asliw", (char *)0, "roxriw", "roril", "asrm", "lsrrw", "roxrrw", 
  };
  static char *MATCH_name_adrm_170[] = {
    "aslil", (char *)0, "roxril", "asrm", "lsrm", "lsrrl", "roxrrl", 
  };
  static char *MATCH_name_adrm_171[] = {
    (char *)0, (char *)0, "asrm", "lsrm", "roxrm", "asrm.ex", "asrm.ex", 
  };
  static char *MATCH_name_adrm_173[] = {
    (char *)0, (char *)0, "lsrm", "roxrm", "rorm", "lsrm.ex", "lsrm.ex", 
  };
  static char *MATCH_name_adrm_175[] = {
    (char *)0, (char *)0, "roxrm", "rorm", "aslrb", "roxrm.ex", "roxrm.ex", 
  };
  static char *MATCH_name_adrm_177[] = {
    (char *)0, (char *)0, "rorm", "rolib", "aslrw", "rorm.ex", "rorm.ex", 
  };
  static char *MATCH_name_adrm_179[] = {
    (char *)0, (char *)0, "roxlib", "roliw", "aslrl", "lslrb", "roxlrb", 
  };
  static char *MATCH_name_adrm_180[] = {
    (char *)0, (char *)0, "roxliw", "rolil", "aslm", "lslrw", "roxlrw", 
  };
  static char *MATCH_name_adrm_181[] = {
    (char *)0, (char *)0, "roxlil", "aslm", "lslm", "lslrl", "roxlrl", 
  };
  static char *MATCH_name_adrm_182[] = {
    (char *)0, (char *)0, "aslm", "lslm", "roxlm", "aslm.ex", "aslm.ex", 
  };
  static char *MATCH_name_adrm_184[] = {
    (char *)0, (char *)0, "lslm", "roxlm", "rolm", "lslm.ex", "lslm.ex", 
  };
  static char *MATCH_name_adrm_186[] = {
    (char *)0, (char *)0, "roxlm", "rolm", (char *)0, "roxlm.ex", "roxlm.ex", 
  };
  static char *MATCH_name_adrm_188[] = {
    (char *)0, (char *)0, "rolm", (char *)0, (char *)0, "rolm.ex", "rolm.ex", 
  };
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  unsigned /* [0..65535] */ MATCH_w_16_32;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */) {
        case 0: 
          
            switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
              case 0: case 2: case 3: case 4: 
                if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
                  
                    switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                      case 0: 
                        { 
                          unsigned ea = addressToPC(MATCH_p);
                          unsigned n = 
                            (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
                          
                          #line 315 "machine/mc68k/decoder_low.m"
                           {

                                  // btst, btsti

                                      int bump = 0, bumpr;

                                      RTs = instantiate(pc, "btst", DIS_DN(32), dBEA(ea, pc, bump, bumpr, 8));

                                      ADDBUMP;

                                  }

                             

                          

                          
                          
                          
                        }
                        
                        break;
                      case 1: case 2: 
                        MATCH_name = 
                          MATCH_name_sz_15[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a5; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_sz_1[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a5; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
                else 
                  
                    switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                      case 0: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "orib"; 
                                goto MATCH_label_a0; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_1[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a2; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_1[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a3; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 1: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "andib"; 
                                goto MATCH_label_a0; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_3[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a2; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_3[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a3; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 2: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "subib"; 
                                goto MATCH_label_a0; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_5[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a2; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_5[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a3; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 3: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "addib"; 
                                goto MATCH_label_a0; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_7[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a2; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_7[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a3; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 4: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                unsigned ea = addressToPC(MATCH_p);
                                unsigned i8 = 
                                  (MATCH_w_16_16 & 0xff) /* disp8 at 16 */;
                                
                                #line 328 "machine/mc68k/decoder_low.m"
                                 { 

                                            int bump = 0, bumpr;

                                            RTs = instantiate (pc, "btst", DIS_I8, dBEA (ea, pc, bump, bumpr, 8)); 

                                            ADDBUMP;

                                            result.numBytes += 2; 

                                        }

                                

                                
                                
                                
                              } /*opt-block*//*opt-block+*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "bchgi"; 
                                goto MATCH_label_a4; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "bclri"; 
                                goto MATCH_label_a4; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 3: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "bseti"; 
                                goto MATCH_label_a4; 
                                
                              } /*opt-block*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 5: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "eorib"; 
                                goto MATCH_label_a0; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_12[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a2; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_12[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a3; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 6: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "cmpib"; 
                                goto MATCH_label_a0; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_14[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a2; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_14[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a3; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 7: 
                        goto MATCH_label_a1; break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/  
                break;
              case 1: 
                goto MATCH_label_a1; break;
              case 5: case 6: 
                if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
                  
                    switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                      case 0: 
                        goto MATCH_label_a11; break;
                      case 1: case 2: 
                        MATCH_name = 
                          MATCH_name_sz_31[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a12; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_sz_3[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a12; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
                else 
                  
                    switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                      case 0: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "orib.ex"; 
                                goto MATCH_label_a6; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_17[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a7; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_17[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a8; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 1: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "andib.ex"; 
                                goto MATCH_label_a6; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_19[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a7; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_19[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a8; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 2: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "subib.ex"; 
                                goto MATCH_label_a6; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_21[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a7; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_21[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a8; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 3: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "addib.ex"; 
                                goto MATCH_label_a6; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_23[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a7; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_23[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a8; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 4: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else 
                                goto MATCH_label_a9;  /*opt-block+*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "bchgi.ex"; 
                                goto MATCH_label_a10; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "bclri.ex"; 
                                goto MATCH_label_a10; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 3: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "bseti.ex"; 
                                goto MATCH_label_a10; 
                                
                              } /*opt-block*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 5: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "eorib.ex"; 
                                goto MATCH_label_a6; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_28[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a7; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_28[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a8; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 6: 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              if ((MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 0 && 
                                (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ && 
                                (MATCH_w_16_16 >> 8 & 0x7) 
                                      /* null at 16 */ < 8) || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 15 & 0x1) 
                                      /* iType at 16 */ == 1 || 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ == 0 && 
                                (MATCH_w_16_16 >> 11 & 0x1) 
                                      /* iSize at 16 */ == 1 || 
                                1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ && 
                                (MATCH_w_16_16 >> 12 & 0x7) 
                                      /* iReg at 16 */ < 8) 
                                goto MATCH_label_a1;  /*opt-block+*/
                              else { 
                                MATCH_name = "cmpib.ex"; 
                                goto MATCH_label_a6; 
                                
                              } /*opt-block*/
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_30[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a7; 
                              
                              break;
                            case 2: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_sz_30[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a8; 
                              
                              break;
                            case 3: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                        break;
                      case 7: 
                        goto MATCH_label_a1; break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/  
                break;
              case 7: 
                
                  switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                    case 0: case 1: 
                      if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
                        
                          switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                            case 0: 
                              goto MATCH_label_a11; break;
                            case 1: case 2: 
                              MATCH_name = 
                                MATCH_name_sz_31[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a12; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_sz_3[(MATCH_w_16_0 >> 6 & 0x3) 
                                    /* sz at 0 */]; 
                              goto MATCH_label_a12; 
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
                      else 
                        
                          switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                            case 0: 
                              
                                switch((MATCH_w_16_0 >> 6 & 0x3) 
                                      /* sz at 0 */) {
                                  case 0: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    if ((MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 0 && 
                                      (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ && 
                                      (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ < 8) || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 1 || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 1 || 
                                      1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ && 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ < 8) 
                                      goto MATCH_label_a1;  /*opt-block+*/
                                    else { 
                                      MATCH_name = "orib.ex"; 
                                      goto MATCH_label_a6; 
                                      
                                    } /*opt-block*/
                                    
                                    break;
                                  case 1: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_17[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a7; 
                                    
                                    break;
                                  case 2: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_17[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a8; 
                                    
                                    break;
                                  case 3: 
                                    goto MATCH_label_a1; break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 >> 6 & 0x3) 
                                      -- sz at 0 --*/ 
                              break;
                            case 1: 
                              
                                switch((MATCH_w_16_0 >> 6 & 0x3) 
                                      /* sz at 0 */) {
                                  case 0: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    if ((MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 0 && 
                                      (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ && 
                                      (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ < 8) || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 1 || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 1 || 
                                      1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ && 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ < 8) 
                                      goto MATCH_label_a1;  /*opt-block+*/
                                    else { 
                                      MATCH_name = "andib.ex"; 
                                      goto MATCH_label_a6; 
                                      
                                    } /*opt-block*/
                                    
                                    break;
                                  case 1: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_19[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a7; 
                                    
                                    break;
                                  case 2: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_19[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a8; 
                                    
                                    break;
                                  case 3: 
                                    goto MATCH_label_a1; break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 >> 6 & 0x3) 
                                      -- sz at 0 --*/ 
                              break;
                            case 2: 
                              
                                switch((MATCH_w_16_0 >> 6 & 0x3) 
                                      /* sz at 0 */) {
                                  case 0: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    if ((MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 0 && 
                                      (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ && 
                                      (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ < 8) || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 1 || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 1 || 
                                      1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ && 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ < 8) 
                                      goto MATCH_label_a1;  /*opt-block+*/
                                    else { 
                                      MATCH_name = "subib.ex"; 
                                      goto MATCH_label_a6; 
                                      
                                    } /*opt-block*/
                                    
                                    break;
                                  case 1: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_21[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a7; 
                                    
                                    break;
                                  case 2: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_21[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a8; 
                                    
                                    break;
                                  case 3: 
                                    goto MATCH_label_a1; break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 >> 6 & 0x3) 
                                      -- sz at 0 --*/ 
                              break;
                            case 3: 
                              
                                switch((MATCH_w_16_0 >> 6 & 0x3) 
                                      /* sz at 0 */) {
                                  case 0: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    if ((MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 0 && 
                                      (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ && 
                                      (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ < 8) || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 1 || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 1 || 
                                      1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ && 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ < 8) 
                                      goto MATCH_label_a1;  /*opt-block+*/
                                    else { 
                                      MATCH_name = "addib.ex"; 
                                      goto MATCH_label_a6; 
                                      
                                    } /*opt-block*/
                                    
                                    break;
                                  case 1: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_23[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a7; 
                                    
                                    break;
                                  case 2: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_23[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a8; 
                                    
                                    break;
                                  case 3: 
                                    goto MATCH_label_a1; break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 >> 6 & 0x3) 
                                      -- sz at 0 --*/ 
                              break;
                            case 4: 
                              
                                switch((MATCH_w_16_0 >> 6 & 0x3) 
                                      /* sz at 0 */) {
                                  case 0: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    if ((MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 0 && 
                                      (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ && 
                                      (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ < 8) || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 1 || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 1 || 
                                      1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ && 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ < 8) 
                                      goto MATCH_label_a1;  /*opt-block+*/
                                    else 
                                      goto MATCH_label_a9;  /*opt-block+*/
                                    
                                    break;
                                  case 1: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    if ((MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 0 && 
                                      (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ && 
                                      (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ < 8) || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 1 || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 1 || 
                                      1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ && 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ < 8) 
                                      goto MATCH_label_a1;  /*opt-block+*/
                                    else { 
                                      MATCH_name = "bchgi.ex"; 
                                      goto MATCH_label_a10; 
                                      
                                    } /*opt-block*/
                                    
                                    break;
                                  case 2: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    if ((MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 0 && 
                                      (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ && 
                                      (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ < 8) || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 1 || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 1 || 
                                      1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ && 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ < 8) 
                                      goto MATCH_label_a1;  /*opt-block+*/
                                    else { 
                                      MATCH_name = "bclri.ex"; 
                                      goto MATCH_label_a10; 
                                      
                                    } /*opt-block*/
                                    
                                    break;
                                  case 3: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    if ((MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 0 && 
                                      (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ && 
                                      (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ < 8) || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 1 || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 1 || 
                                      1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ && 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ < 8) 
                                      goto MATCH_label_a1;  /*opt-block+*/
                                    else { 
                                      MATCH_name = "bseti.ex"; 
                                      goto MATCH_label_a10; 
                                      
                                    } /*opt-block*/
                                    
                                    break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 >> 6 & 0x3) 
                                      -- sz at 0 --*/ 
                              break;
                            case 5: 
                              
                                switch((MATCH_w_16_0 >> 6 & 0x3) 
                                      /* sz at 0 */) {
                                  case 0: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    if ((MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 0 && 
                                      (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ && 
                                      (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ < 8) || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 1 || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 1 || 
                                      1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ && 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ < 8) 
                                      goto MATCH_label_a1;  /*opt-block+*/
                                    else { 
                                      MATCH_name = "eorib.ex"; 
                                      goto MATCH_label_a6; 
                                      
                                    } /*opt-block*/
                                    
                                    break;
                                  case 1: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_28[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a7; 
                                    
                                    break;
                                  case 2: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_28[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a8; 
                                    
                                    break;
                                  case 3: 
                                    goto MATCH_label_a1; break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 >> 6 & 0x3) 
                                      -- sz at 0 --*/ 
                              break;
                            case 6: 
                              
                                switch((MATCH_w_16_0 >> 6 & 0x3) 
                                      /* sz at 0 */) {
                                  case 0: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    if ((MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 0 && 
                                      (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ && 
                                      (MATCH_w_16_16 >> 8 & 0x7) 
                                            /* null at 16 */ < 8) || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 15 & 0x1) 
                                            /* iType at 16 */ == 1 || 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ == 0 && 
                                      (MATCH_w_16_16 >> 11 & 0x1) 
                                            /* iSize at 16 */ == 1 || 
                                      1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ && 
                                      (MATCH_w_16_16 >> 12 & 0x7) 
                                            /* iReg at 16 */ < 8) 
                                      goto MATCH_label_a1;  /*opt-block+*/
                                    else { 
                                      MATCH_name = "cmpib.ex"; 
                                      goto MATCH_label_a6; 
                                      
                                    } /*opt-block*/
                                    
                                    break;
                                  case 1: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_30[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a7; 
                                    
                                    break;
                                  case 2: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                    MATCH_name = 
                                      MATCH_name_sz_30[(MATCH_w_16_0 >> 6 & 0x3) 
                                          /* sz at 0 */]; 
                                    goto MATCH_label_a8; 
                                    
                                    break;
                                  case 3: 
                                    goto MATCH_label_a1; break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 >> 6 & 0x3) 
                                      -- sz at 0 --*/ 
                              break;
                            case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/  
                      break;
                    case 2: case 3: 
                      if ((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 0) 
                        if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
                          goto MATCH_label_a11;  /*opt-block+*/
                        else { 
                          MATCH_w_16_16 = getWord(2 + MATCH_p); 
                          if (0 <= (MATCH_w_16_0 >> 9 & 0x7) 
                                  /* reg1 at 0 */ && 
                            (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 4 || 
                            5 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
                            (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 8 || 
                            (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 4 && 
                            (MATCH_w_16_16 >> 12 & 0x7) 
                                  /* iReg at 16 */ == 0 && 
                            (MATCH_w_16_16 >> 11 & 0x1) 
                                  /* iSize at 16 */ == 0 && 
                            (MATCH_w_16_16 >> 15 & 0x1) 
                                  /* iType at 16 */ == 0 && 
                            (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                  /* null at 16 */ && 
                            (MATCH_w_16_16 >> 8 & 0x7) 
                                  /* null at 16 */ < 8) || 
                            (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 4 && 
                            (MATCH_w_16_16 >> 12 & 0x7) 
                                  /* iReg at 16 */ == 0 && 
                            (MATCH_w_16_16 >> 11 & 0x1) 
                                  /* iSize at 16 */ == 0 && 
                            (MATCH_w_16_16 >> 15 & 0x1) 
                                  /* iType at 16 */ == 1 || 
                            (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 4 && 
                            (MATCH_w_16_16 >> 12 & 0x7) 
                                  /* iReg at 16 */ == 0 && 
                            (MATCH_w_16_16 >> 11 & 0x1) 
                                  /* iSize at 16 */ == 1 || 
                            (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 4 && 
                            (1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                  /* iReg at 16 */ && 
                            (MATCH_w_16_16 >> 12 & 0x7) 
                                  /* iReg at 16 */ < 8)) 
                            goto MATCH_label_a1;  /*opt-block+*/
                          else 
                            goto MATCH_label_a9;  /*opt-block+*/
                          
                        } /*opt-block*/ 
                      else 
                        goto MATCH_label_a1;  /*opt-block+*/
                      break;
                    case 4: 
                      if ((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 0) 
                        if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
                          goto MATCH_label_a11;  /*opt-block+*/
                        else 
                          
                            switch((MATCH_w_16_0 >> 9 & 0x7) 
                                  /* reg1 at 0 */) {
                              case 0: 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                if ((MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 15 & 0x1) 
                                        /* iType at 16 */ == 0 && 
                                  (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                        /* null at 16 */ && 
                                  (MATCH_w_16_16 >> 8 & 0x7) 
                                        /* null at 16 */ < 8) || 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 15 & 0x1) 
                                        /* iType at 16 */ == 1 || 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 1 || 
                                  1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ && 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ < 8) 
                                  goto MATCH_label_a1;  /*opt-block+*/
                                else { 
                                  MATCH_name = "oriToCCR"; 
                                  goto MATCH_label_a13; 
                                  
                                } /*opt-block*/
                                
                                break;
                              case 1: 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                if ((MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 15 & 0x1) 
                                        /* iType at 16 */ == 0 && 
                                  (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                        /* null at 16 */ && 
                                  (MATCH_w_16_16 >> 8 & 0x7) 
                                        /* null at 16 */ < 8) || 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 15 & 0x1) 
                                        /* iType at 16 */ == 1 || 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 1 || 
                                  1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ && 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ < 8) 
                                  goto MATCH_label_a1;  /*opt-block+*/
                                else { 
                                  MATCH_name = "andiToCCR"; 
                                  goto MATCH_label_a13; 
                                  
                                } /*opt-block*/
                                
                                break;
                              case 2: case 3: case 6: case 7: 
                                goto MATCH_label_a1; break;
                              case 4: 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                if ((MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 15 & 0x1) 
                                        /* iType at 16 */ == 0 && 
                                  (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                        /* null at 16 */ && 
                                  (MATCH_w_16_16 >> 8 & 0x7) 
                                        /* null at 16 */ < 8) || 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 15 & 0x1) 
                                        /* iType at 16 */ == 1 || 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 1 || 
                                  1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ && 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ < 8) 
                                  goto MATCH_label_a1;  /*opt-block+*/
                                else 
                                  goto MATCH_label_a9;  /*opt-block+*/
                                
                                break;
                              case 5: 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                if ((MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 15 & 0x1) 
                                        /* iType at 16 */ == 0 && 
                                  (1 <= (MATCH_w_16_16 >> 8 & 0x7) 
                                        /* null at 16 */ && 
                                  (MATCH_w_16_16 >> 8 & 0x7) 
                                        /* null at 16 */ < 8) || 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 15 & 0x1) 
                                        /* iType at 16 */ == 1 || 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ == 0 && 
                                  (MATCH_w_16_16 >> 11 & 0x1) 
                                        /* iSize at 16 */ == 1 || 
                                  1 <= (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ && 
                                  (MATCH_w_16_16 >> 12 & 0x7) 
                                        /* iReg at 16 */ < 8) 
                                  goto MATCH_label_a1;  /*opt-block+*/
                                else { 
                                  MATCH_name = "eoriToCCR"; 
                                  goto MATCH_label_a13; 
                                  
                                } /*opt-block*/
                                
                                break;
                              default: assert(0);
                            } /* (MATCH_w_16_0 >> 9 & 0x7) 
                                  -- reg1 at 0 --*/   
                      else 
                        goto MATCH_label_a1;  /*opt-block+*/
                      break;
                    case 5: case 6: case 7: 
                      goto MATCH_label_a1; break;
                    default: assert(0);
                  } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                break;
              default: assert(0);
            } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
          break;
        case 1: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
            if ((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) 
              if (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
                (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 2) 
                
                  switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                    case 0: case 1: case 2: case 3: case 4: 
                      if (5 <= (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ && 
                        (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ < 8) { 
                        MATCH_name = 
                          MATCH_name_MDadrm_39[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a17; 
                        
                      } /*opt-block*/
                      else { 
                        MATCH_name = 
                          MATCH_name_MDadrm_39[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a14; 
                        
                      } /*opt-block*/
                      
                      break;
                    case 5: case 6: 
                      if (5 <= (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ && 
                        (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ < 8) { 
                        MATCH_name = 
                          MATCH_name_MDadrm_40[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a18; 
                        
                      } /*opt-block*/
                      else { 
                        MATCH_name = 
                          MATCH_name_MDadrm_40[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a15; 
                        
                      } /*opt-block*/
                      
                      break;
                    case 7: 
                      
                        switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                          case 0: case 2: case 3: case 4: 
                            if (5 <= (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ && 
                              (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ < 8) { 
                              MATCH_name = 
                                MATCH_name_MDadrm_40[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a18; 
                              
                            } /*opt-block*/
                            else { 
                              MATCH_name = 
                                MATCH_name_MDadrm_40[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a15; 
                              
                            } /*opt-block*/
                            
                            break;
                          case 1: 
                            if (5 <= (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ && 
                              (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ < 8) { 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_MDadrm_41[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a19; 
                              
                            } /*opt-block*/
                            else { 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_MDadrm_41[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a16; 
                              
                            } /*opt-block*/
                            
                            break;
                          case 5: case 6: case 7: 
                            goto MATCH_label_a1; break;
                          default: assert(0);
                        } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                      break;
                    default: assert(0);
                  } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
              else 
                
                  switch((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */) {
                    case 0: case 1: case 2: case 3: case 4: 
                      
                        switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                          case 0: case 1: case 2: case 3: case 4: 
                            MATCH_name = 
                              MATCH_name_adrm_35[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a14; 
                            
                            break;
                          case 5: case 6: 
                            MATCH_name = 
                              MATCH_name_adrm_35[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a15; 
                            
                            break;
                          case 7: 
                            
                              switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                                case 0: case 2: case 3: case 4: 
                                  MATCH_name = 
                                    MATCH_name_reg2_36[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a15; 
                                  
                                  break;
                                case 1: 
                                  MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                  MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                  MATCH_name = 
                                    MATCH_name_reg2_36[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a16; 
                                  
                                  break;
                                case 5: case 6: case 7: 
                                  goto MATCH_label_a1; break;
                                default: assert(0);
                              } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                            break;
                          default: assert(0);
                        } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                      break;
                    case 5: case 6: 
                      
                        switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                          case 0: case 1: case 2: case 3: case 4: 
                            MATCH_name = 
                              MATCH_name_adrm_37[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a17; 
                            
                            break;
                          case 5: case 6: 
                            MATCH_name = 
                              MATCH_name_adrm_37[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a18; 
                            
                            break;
                          case 7: 
                            
                              switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                                case 0: case 2: case 3: case 4: 
                                  MATCH_name = 
                                    MATCH_name_reg2_38[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a18; 
                                  
                                  break;
                                case 1: 
                                  MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                  MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                  MATCH_name = 
                                    MATCH_name_reg2_38[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a19; 
                                  
                                  break;
                                case 5: case 6: case 7: 
                                  goto MATCH_label_a1; break;
                                default: assert(0);
                              } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                            break;
                          default: assert(0);
                        } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                      break;
                    case 7: 
                      goto MATCH_label_a1; break;
                    default: assert(0);
                  } /* (MATCH_w_16_0 >> 6 & 0x7) -- MDadrm at 0 --*/   
            else 
              
                switch((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */) {
                  case 0: case 1: case 2: case 3: case 4: 
                    
                      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                        case 0: case 1: case 2: case 3: case 4: 
                          MATCH_name = 
                            MATCH_name_adrm_35[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a14; 
                          
                          break;
                        case 5: case 6: 
                          MATCH_name = 
                            MATCH_name_adrm_35[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a15; 
                          
                          break;
                        case 7: 
                          
                            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                              case 0: case 2: case 3: case 4: 
                                MATCH_name = 
                                  MATCH_name_reg2_36[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a15; 
                                
                                break;
                              case 1: 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                MATCH_name = 
                                  MATCH_name_reg2_36[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a16; 
                                
                                break;
                              case 5: case 6: case 7: 
                                goto MATCH_label_a1; break;
                              default: assert(0);
                            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                          break;
                        default: assert(0);
                      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                    break;
                  case 5: case 6: 
                    
                      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                        case 0: case 1: case 2: case 3: case 4: 
                          MATCH_name = 
                            MATCH_name_adrm_37[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a17; 
                          
                          break;
                        case 5: case 6: 
                          MATCH_name = 
                            MATCH_name_adrm_37[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a18; 
                          
                          break;
                        case 7: 
                          
                            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                              case 0: case 2: case 3: case 4: 
                                MATCH_name = 
                                  MATCH_name_reg2_38[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a18; 
                                
                                break;
                              case 1: 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                MATCH_name = 
                                  MATCH_name_reg2_38[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a19; 
                                
                                break;
                              case 5: case 6: case 7: 
                                goto MATCH_label_a1; break;
                              default: assert(0);
                            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                          break;
                        default: assert(0);
                      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                    break;
                  case 7: 
                    goto MATCH_label_a1; break;
                  default: assert(0);
                } /* (MATCH_w_16_0 >> 6 & 0x7) -- MDadrm at 0 --*/   
          else 
            
              switch((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */) {
                case 0: case 1: case 2: case 3: case 4: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 1: case 2: case 3: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_35[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a14; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_35[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a15; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 2: case 3: case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_36[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a15; 
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_reg2_36[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a16; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 5: case 6: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 1: case 2: case 3: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_37[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a17; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_37[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a18; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 2: case 3: case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_38[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a18; 
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_reg2_38[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a19; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 7: 
                  goto MATCH_label_a1; break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x7) -- MDadrm at 0 --*/  
          break;
        case 2: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
            if ((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) 
              if (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
                (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 2) 
                
                  switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                    case 0: case 1: case 2: case 3: case 4: 
                      if (5 <= (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ && 
                        (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ < 8) { 
                        MATCH_name = 
                          MATCH_name_MDadrm_46[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a17; 
                        
                      } /*opt-block*/
                      else { 
                        MATCH_name = 
                          MATCH_name_MDadrm_46[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a14; 
                        
                      } /*opt-block*/
                      
                      break;
                    case 5: case 6: 
                      if (5 <= (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ && 
                        (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ < 8) { 
                        MATCH_name = 
                          MATCH_name_MDadrm_47[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a18; 
                        
                      } /*opt-block*/
                      else { 
                        MATCH_name = 
                          MATCH_name_MDadrm_47[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a15; 
                        
                      } /*opt-block*/
                      
                      break;
                    case 7: 
                      
                        switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                          case 0: case 2: case 3: 
                            if (5 <= (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ && 
                              (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ < 8) { 
                              MATCH_name = 
                                MATCH_name_MDadrm_47[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a18; 
                              
                            } /*opt-block*/
                            else { 
                              MATCH_name = 
                                MATCH_name_MDadrm_47[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a15; 
                              
                            } /*opt-block*/
                            
                            break;
                          case 1: case 4: 
                            if (5 <= (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ && 
                              (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ < 8) { 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_MDadrm_48[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a19; 
                              
                            } /*opt-block*/
                            else { 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_MDadrm_48[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a16; 
                              
                            } /*opt-block*/
                            
                            break;
                          case 5: case 6: case 7: 
                            goto MATCH_label_a1; break;
                          default: assert(0);
                        } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                      break;
                    default: assert(0);
                  } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
              else 
                
                  switch((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */) {
                    case 0: case 1: case 2: case 3: case 4: 
                      
                        switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                          case 0: case 1: case 2: case 3: case 4: 
                            MATCH_name = 
                              MATCH_name_adrm_42[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a14; 
                            
                            break;
                          case 5: case 6: 
                            MATCH_name = 
                              MATCH_name_adrm_42[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a15; 
                            
                            break;
                          case 7: 
                            
                              switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                                case 0: case 2: case 3: 
                                  MATCH_name = 
                                    MATCH_name_reg2_43[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a15; 
                                  
                                  break;
                                case 1: case 4: 
                                  MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                  MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                  MATCH_name = 
                                    MATCH_name_reg2_43[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a16; 
                                  
                                  break;
                                case 5: case 6: case 7: 
                                  goto MATCH_label_a1; break;
                                default: assert(0);
                              } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                            break;
                          default: assert(0);
                        } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                      break;
                    case 5: case 6: 
                      
                        switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                          case 0: case 1: case 2: case 3: case 4: 
                            MATCH_name = 
                              MATCH_name_adrm_44[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a17; 
                            
                            break;
                          case 5: case 6: 
                            MATCH_name = 
                              MATCH_name_adrm_44[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a18; 
                            
                            break;
                          case 7: 
                            
                              switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                                case 0: case 2: case 3: 
                                  MATCH_name = 
                                    MATCH_name_reg2_45[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a18; 
                                  
                                  break;
                                case 1: case 4: 
                                  MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                  MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                  MATCH_name = 
                                    MATCH_name_reg2_45[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a19; 
                                  
                                  break;
                                case 5: case 6: case 7: 
                                  goto MATCH_label_a1; break;
                                default: assert(0);
                              } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                            break;
                          default: assert(0);
                        } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                      break;
                    case 7: 
                      goto MATCH_label_a1; break;
                    default: assert(0);
                  } /* (MATCH_w_16_0 >> 6 & 0x7) -- MDadrm at 0 --*/   
            else 
              
                switch((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */) {
                  case 0: case 1: case 2: case 3: case 4: 
                    
                      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                        case 0: case 1: case 2: case 3: case 4: 
                          MATCH_name = 
                            MATCH_name_adrm_42[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a14; 
                          
                          break;
                        case 5: case 6: 
                          MATCH_name = 
                            MATCH_name_adrm_42[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a15; 
                          
                          break;
                        case 7: 
                          
                            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                              case 0: case 2: case 3: 
                                MATCH_name = 
                                  MATCH_name_reg2_43[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a15; 
                                
                                break;
                              case 1: case 4: 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                MATCH_name = 
                                  MATCH_name_reg2_43[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a16; 
                                
                                break;
                              case 5: case 6: case 7: 
                                goto MATCH_label_a1; break;
                              default: assert(0);
                            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                          break;
                        default: assert(0);
                      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                    break;
                  case 5: case 6: 
                    
                      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                        case 0: case 1: case 2: case 3: case 4: 
                          MATCH_name = 
                            MATCH_name_adrm_44[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a17; 
                          
                          break;
                        case 5: case 6: 
                          MATCH_name = 
                            MATCH_name_adrm_44[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a18; 
                          
                          break;
                        case 7: 
                          
                            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                              case 0: case 2: case 3: 
                                MATCH_name = 
                                  MATCH_name_reg2_45[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a18; 
                                
                                break;
                              case 1: case 4: 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                MATCH_name = 
                                  MATCH_name_reg2_45[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a19; 
                                
                                break;
                              case 5: case 6: case 7: 
                                goto MATCH_label_a1; break;
                              default: assert(0);
                            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                          break;
                        default: assert(0);
                      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                    break;
                  case 7: 
                    goto MATCH_label_a1; break;
                  default: assert(0);
                } /* (MATCH_w_16_0 >> 6 & 0x7) -- MDadrm at 0 --*/   
          else 
            
              switch((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */) {
                case 0: case 1: case 2: case 3: case 4: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 1: case 2: case 3: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_42[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a14; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_42[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a15; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 2: case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_43[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a15; 
                              
                              break;
                            case 1: case 4: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_reg2_43[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a16; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 5: case 6: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 1: case 2: case 3: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_44[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a17; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_44[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a18; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 2: case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_45[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a18; 
                              
                              break;
                            case 1: case 4: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_reg2_45[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a19; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 7: 
                  goto MATCH_label_a1; break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x7) -- MDadrm at 0 --*/  
          break;
        case 3: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
            if ((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) 
              if (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
                (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 2) 
                
                  switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                    case 0: case 1: case 2: case 3: case 4: 
                      if (5 <= (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ && 
                        (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ < 8) { 
                        MATCH_name = 
                          MATCH_name_MDadrm_53[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a17; 
                        
                      } /*opt-block*/
                      else { 
                        MATCH_name = 
                          MATCH_name_MDadrm_53[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a14; 
                        
                      } /*opt-block*/
                      
                      break;
                    case 5: case 6: 
                      if (5 <= (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ && 
                        (MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */ < 8) { 
                        MATCH_name = 
                          MATCH_name_MDadrm_54[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a18; 
                        
                      } /*opt-block*/
                      else { 
                        MATCH_name = 
                          MATCH_name_MDadrm_54[(MATCH_w_16_0 >> 6 & 0x7) 
                              /* MDadrm at 0 */]; 
                        goto MATCH_label_a15; 
                        
                      } /*opt-block*/
                      
                      break;
                    case 7: 
                      
                        switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                          case 0: case 2: case 3: case 4: 
                            if (5 <= (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ && 
                              (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ < 8) { 
                              MATCH_name = 
                                MATCH_name_MDadrm_54[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a18; 
                              
                            } /*opt-block*/
                            else { 
                              MATCH_name = 
                                MATCH_name_MDadrm_54[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a15; 
                              
                            } /*opt-block*/
                            
                            break;
                          case 1: 
                            if (5 <= (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ && 
                              (MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */ < 8) { 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_MDadrm_55[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a19; 
                              
                            } /*opt-block*/
                            else { 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_MDadrm_55[(MATCH_w_16_0 >> 6 & 0x7) 
                                    /* MDadrm at 0 */]; 
                              goto MATCH_label_a16; 
                              
                            } /*opt-block*/
                            
                            break;
                          case 5: case 6: case 7: 
                            goto MATCH_label_a1; break;
                          default: assert(0);
                        } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                      break;
                    default: assert(0);
                  } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
              else 
                
                  switch((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */) {
                    case 0: case 1: case 2: case 3: case 4: 
                      
                        switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                          case 0: case 1: case 2: case 3: case 4: 
                            MATCH_name = 
                              MATCH_name_adrm_49[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a14; 
                            
                            break;
                          case 5: case 6: 
                            MATCH_name = 
                              MATCH_name_adrm_49[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a15; 
                            
                            break;
                          case 7: 
                            
                              switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                                case 0: case 2: case 3: case 4: 
                                  MATCH_name = 
                                    MATCH_name_reg2_50[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a15; 
                                  
                                  break;
                                case 1: 
                                  MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                  MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                  MATCH_name = 
                                    MATCH_name_reg2_50[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a16; 
                                  
                                  break;
                                case 5: case 6: case 7: 
                                  goto MATCH_label_a1; break;
                                default: assert(0);
                              } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                            break;
                          default: assert(0);
                        } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                      break;
                    case 5: case 6: 
                      
                        switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                          case 0: case 1: case 2: case 3: case 4: 
                            MATCH_name = 
                              MATCH_name_adrm_51[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a17; 
                            
                            break;
                          case 5: case 6: 
                            MATCH_name = 
                              MATCH_name_adrm_51[(MATCH_w_16_0 >> 3 & 0x7) 
                                  /* adrm at 0 */]; 
                            goto MATCH_label_a18; 
                            
                            break;
                          case 7: 
                            
                              switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                                case 0: case 2: case 3: case 4: 
                                  MATCH_name = 
                                    MATCH_name_reg2_52[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a18; 
                                  
                                  break;
                                case 1: 
                                  MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                  MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                  MATCH_name = 
                                    MATCH_name_reg2_52[(MATCH_w_16_0 & 0x7) 
                                        /* reg2 at 0 */]; 
                                  goto MATCH_label_a19; 
                                  
                                  break;
                                case 5: case 6: case 7: 
                                  goto MATCH_label_a1; break;
                                default: assert(0);
                              } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                            break;
                          default: assert(0);
                        } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                      break;
                    case 7: 
                      goto MATCH_label_a1; break;
                    default: assert(0);
                  } /* (MATCH_w_16_0 >> 6 & 0x7) -- MDadrm at 0 --*/   
            else 
              
                switch((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */) {
                  case 0: case 1: case 2: case 3: case 4: 
                    
                      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                        case 0: case 1: case 2: case 3: case 4: 
                          MATCH_name = 
                            MATCH_name_adrm_49[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a14; 
                          
                          break;
                        case 5: case 6: 
                          MATCH_name = 
                            MATCH_name_adrm_49[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a15; 
                          
                          break;
                        case 7: 
                          
                            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                              case 0: case 2: case 3: case 4: 
                                MATCH_name = 
                                  MATCH_name_reg2_50[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a15; 
                                
                                break;
                              case 1: 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                MATCH_name = 
                                  MATCH_name_reg2_50[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a16; 
                                
                                break;
                              case 5: case 6: case 7: 
                                goto MATCH_label_a1; break;
                              default: assert(0);
                            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                          break;
                        default: assert(0);
                      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                    break;
                  case 5: case 6: 
                    
                      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                        case 0: case 1: case 2: case 3: case 4: 
                          MATCH_name = 
                            MATCH_name_adrm_51[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a17; 
                          
                          break;
                        case 5: case 6: 
                          MATCH_name = 
                            MATCH_name_adrm_51[(MATCH_w_16_0 >> 3 & 0x7) 
                                /* adrm at 0 */]; 
                          goto MATCH_label_a18; 
                          
                          break;
                        case 7: 
                          
                            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                              case 0: case 2: case 3: case 4: 
                                MATCH_name = 
                                  MATCH_name_reg2_52[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a18; 
                                
                                break;
                              case 1: 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                MATCH_w_16_32 = getWord(4 + MATCH_p); 
                                MATCH_name = 
                                  MATCH_name_reg2_52[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a19; 
                                
                                break;
                              case 5: case 6: case 7: 
                                goto MATCH_label_a1; break;
                              default: assert(0);
                            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                          break;
                        default: assert(0);
                      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                    break;
                  case 7: 
                    goto MATCH_label_a1; break;
                  default: assert(0);
                } /* (MATCH_w_16_0 >> 6 & 0x7) -- MDadrm at 0 --*/   
          else 
            
              switch((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */) {
                case 0: case 1: case 2: case 3: case 4: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 1: case 2: case 3: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_49[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a14; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_49[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a15; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 2: case 3: case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_50[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a15; 
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_reg2_50[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a16; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 5: case 6: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 1: case 2: case 3: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_51[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a17; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_51[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a18; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 2: case 3: case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_52[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a18; 
                              
                              break;
                            case 1: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              MATCH_w_16_32 = getWord(4 + MATCH_p); 
                              MATCH_name = 
                                MATCH_name_reg2_52[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a19; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 7: 
                  goto MATCH_label_a1; break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x7) -- MDadrm at 0 --*/  
          break;
        case 4: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: case 1: 
                  goto MATCH_label_a1; break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_82[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a41; 
                        
                        break;
                      case 1: 
                        goto MATCH_label_a1; break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_79[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a41; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_78[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a41; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_82[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a42; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = 
                                MATCH_name_reg2_83[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a42; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_65[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a42; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_67[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a42; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_63[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a42; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 1: case 3: case 4: 
                        goto MATCH_label_a1; break;
                      case 2: 
                        { 
                          unsigned ea = addressToPC(MATCH_p);
                          unsigned n = 
                            (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
                          
                          #line 619 "machine/mc68k/decoder_low.m"
                           { 

                                  // lea    

                                      RTs = instantiate (pc, "lea", cEA (ea, pc, 32), DIS_AN); 

                                  }  

                          

                          
                          
                          
                        }
                        
                        break;
                      case 5: case 6: 
                        goto MATCH_label_a43; break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 4) 
                          goto MATCH_label_a43;  /*opt-block+*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          else 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 2: case 3: case 4: 
                        if (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
                          (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 6) { 
                          MATCH_name = 
                            MATCH_name_reg1_56[(MATCH_w_16_0 >> 9 & 0x7) 
                                /* reg1 at 0 */]; 
                          goto MATCH_label_a20; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      case 1: 
                        goto MATCH_label_a1; break;
                      case 5: case 6: 
                        if (0 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
                          (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 6) { 
                          MATCH_name = 
                            MATCH_name_reg1_57[(MATCH_w_16_0 >> 9 & 0x7) 
                                /* reg1 at 0 */]; 
                          goto MATCH_label_a21; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2 && 
                          (6 <= (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ && 
                          (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ < 8) || 
                          2 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 8) 
                          goto MATCH_label_a1;  /*opt-block+*/
                        else { 
                          MATCH_name = 
                            MATCH_name_reg1_57[(MATCH_w_16_0 >> 9 & 0x7) 
                                /* reg1 at 0 */]; 
                          goto MATCH_label_a21; 
                          
                        } /*opt-block*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 1: 
                  
                    switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                      case 0: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 2: case 3: case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_58[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a22; 
                              
                              break;
                            case 1: 
                              goto MATCH_label_a1; break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_58[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a23; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = 
                                  MATCH_name_reg2_59[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a23; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 1: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 2: case 3: case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_60[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a22; 
                              
                              break;
                            case 1: 
                              goto MATCH_label_a1; break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_60[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a23; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = 
                                  MATCH_name_reg2_61[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a23; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 2: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 2: case 3: case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_62[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a22; 
                              
                              break;
                            case 1: 
                              goto MATCH_label_a1; break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_62[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a23; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = 
                                  MATCH_name_reg2_63[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a23; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 3: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 2: case 3: case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_64[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a22; 
                              
                              break;
                            case 1: 
                              goto MATCH_label_a1; break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_64[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a23; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = 
                                  MATCH_name_reg2_65[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a23; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 4: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: 
                              MATCH_name = 
                                MATCH_name_adrm_66[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a24; 
                              
                              break;
                            case 1: case 3: case 4: 
                              goto MATCH_label_a1; break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_adrm_66[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a25; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_66[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a26; 
                              
                              break;
                            case 7: 
                              
                                switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                                  case 0: case 1: 
                                    MATCH_name = 
                                      MATCH_name_reg2_67[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a26; 
                                    
                                    break;
                                  case 2: case 3: 
                                    MATCH_name = 
                                      MATCH_name_reg2_59[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a26; 
                                    
                                    break;
                                  case 4: case 5: case 6: case 7: 
                                    goto MATCH_label_a1; break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 5: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 2: 
                              MATCH_name = 
                                MATCH_name_adrm_68[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a22; 
                              
                              break;
                            case 1: 
                              goto MATCH_label_a1; break;
                            case 3: case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_66[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a22; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_68[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a23; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = 
                                  MATCH_name_reg2_69[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a23; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 6: 
                        goto MATCH_label_a1; break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 1: case 4: case 5: case 7: 
                              if ((MATCH_w_16_0 >> 4 & 0x3) 
                                      /* adrb at 0 */ == 0) 
                                goto MATCH_label_a27;  /*opt-block+*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            case 2: 
                              if ((MATCH_w_16_0 >> 4 & 0x3) 
                                      /* adrb at 0 */ == 0) 
                                goto MATCH_label_a27;  /*opt-block+*/
                              else { 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                MATCH_name = "link"; 
                                { 
                                  char *name = MATCH_name;
                                  int /* [~32768..32767] */ i16 = 
                                    sign_extend(
                                                (MATCH_w_16_16 & 0xffff) 
                                                      /* d16 at 16 */, 16);
                                  unsigned n = 
                                    (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
                                  
                                  #line 549 "machine/mc68k/decoder_low.m"
                                   { 

                                          // link

                                              RTs = instantiate (pc, name, DIS_AN, DIS_I16); 

                                              result.numBytes += 2; 

                                          // moveFromUSP, moveToUSP privileged

                                                      }

                                      

                                  
                                  
                                  
                                }
                                
                              } /*opt-block*/
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_68[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              { 
                                char *name = MATCH_name;
                                unsigned n = 
                                  (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
                                
                                #line 499 "machine/mc68k/decoder_low.m"
                                 { 

                                        // unlk

                                            RTs = instantiate (pc, name, DIS_AN); 

                                        }

                                    

                                
                                
                                
                              }
                              
                              break;
                            case 6: 
                              
                                switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                                  case 0: case 1: 
                                    MATCH_name = 
                                      MATCH_name_reg2_71[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a28; 
                                    
                                    break;
                                  case 2: case 4: 
                                    if ((MATCH_w_16_0 >> 4 & 0x3) 
                                            /* adrb at 0 */ == 0) 
                                      goto MATCH_label_a27;  /*opt-block+*/
                                    else 
                                      goto MATCH_label_a1;  /*opt-block+*/
                                    
                                    break;
                                  case 3: 
                                    MATCH_name = 
                                      MATCH_name_reg2_61[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a28; 
                                    
                                    break;
                                  case 5: case 6: case 7: 
                                    MATCH_name = 
                                      MATCH_name_reg2_36[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a28; 
                                    
                                    break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/ 
                  break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        
                          switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                            case 0: case 1: case 2: case 3: case 5: 
                              MATCH_name = 
                                MATCH_name_reg1_72[(MATCH_w_16_0 >> 9 & 0x7) 
                                    /* reg1 at 0 */]; 
                              goto MATCH_label_a29; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg1_72[(MATCH_w_16_0 >> 9 & 0x7) 
                                    /* reg1 at 0 */]; 
                              goto MATCH_label_a24; 
                              
                              break;
                            case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/ 
                        break;
                      case 1: 
                        goto MATCH_label_a1; break;
                      case 2: 
                        
                          switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                            case 0: case 1: case 2: case 3: case 5: 
                              MATCH_name = 
                                MATCH_name_reg1_72[(MATCH_w_16_0 >> 9 & 0x7) 
                                    /* reg1 at 0 */]; 
                              goto MATCH_label_a29; 
                              
                              break;
                            case 4: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              goto MATCH_label_a30; 
                              
                              break;
                            case 6: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              goto MATCH_label_a31; 
                              
                              break;
                            case 7: 
                              MATCH_name = 
                                MATCH_name_reg1_56[(MATCH_w_16_0 >> 9 & 0x7) 
                                    /* reg1 at 0 */]; 
                              goto MATCH_label_a25; 
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/ 
                        break;
                      case 3: 
                        
                          switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                            case 0: case 1: case 2: case 3: case 5: 
                              MATCH_name = 
                                MATCH_name_reg1_72[(MATCH_w_16_0 >> 9 & 0x7) 
                                    /* reg1 at 0 */]; 
                              goto MATCH_label_a29; 
                              
                              break;
                            case 4: case 7: 
                              goto MATCH_label_a1; break;
                            case 6: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              goto MATCH_label_a31; 
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/ 
                        break;
                      case 4: 
                        
                          switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                            case 0: case 1: case 2: case 3: case 5: 
                              MATCH_name = 
                                MATCH_name_reg1_72[(MATCH_w_16_0 >> 9 & 0x7) 
                                    /* reg1 at 0 */]; 
                              goto MATCH_label_a29; 
                              
                              break;
                            case 4: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              goto MATCH_label_a30; 
                              
                              break;
                            case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/ 
                        break;
                      case 5: case 6: 
                        
                          switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                            case 0: case 1: case 2: case 3: case 5: 
                              MATCH_name = 
                                MATCH_name_reg1_73[(MATCH_w_16_0 >> 9 & 0x7) 
                                    /* reg1 at 0 */]; 
                              goto MATCH_label_a32; 
                              
                              break;
                            case 4: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              goto MATCH_label_a33; 
                              
                              break;
                            case 6: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              goto MATCH_label_a34; 
                              
                              break;
                            case 7: 
                              MATCH_name = 
                                MATCH_name_reg1_57[(MATCH_w_16_0 >> 9 & 0x7) 
                                    /* reg1 at 0 */]; 
                              goto MATCH_label_a26; 
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/ 
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              
                                switch((MATCH_w_16_0 >> 9 & 0x7) 
                                      /* reg1 at 0 */) {
                                  case 0: case 1: case 2: case 3: case 5: 
                                    MATCH_name = 
                                      MATCH_name_reg1_73[(MATCH_w_16_0 >> 9 & 0x7) 
                                          /* reg1 at 0 */]; 
                                    goto MATCH_label_a32; 
                                    
                                    break;
                                  case 4: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    goto MATCH_label_a33; 
                                    
                                    break;
                                  case 6: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    goto MATCH_label_a34; 
                                    
                                    break;
                                  case 7: 
                                    MATCH_name = 
                                      MATCH_name_reg1_57[(MATCH_w_16_0 >> 9 & 0x7) 
                                          /* reg1 at 0 */]; 
                                    goto MATCH_label_a26; 
                                    
                                    break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 >> 9 & 0x7) 
                                      -- reg1 at 0 --*/ 
                              break;
                            case 2: case 3: 
                              
                                switch((MATCH_w_16_0 >> 9 & 0x7) 
                                      /* reg1 at 0 */) {
                                  case 0: case 1: case 2: case 3: case 4: 
                                  case 5: 
                                    goto MATCH_label_a1; break;
                                  case 6: 
                                    MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                    goto MATCH_label_a34; 
                                    
                                    break;
                                  case 7: 
                                    MATCH_name = 
                                      MATCH_name_reg1_57[(MATCH_w_16_0 >> 9 & 0x7) 
                                          /* reg1 at 0 */]; 
                                    goto MATCH_label_a26; 
                                    
                                    break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 >> 9 & 0x7) 
                                      -- reg1 at 0 --*/ 
                              break;
                            case 4: case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                      case 0: case 3: 
                        goto MATCH_label_a1; break;
                      case 1: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 2: case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_74[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a35; 
                              
                              break;
                            case 1: 
                              goto MATCH_label_a1; break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_68[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a35; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_74[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a36; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = 
                                  MATCH_name_reg2_75[(MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */]; 
                                goto MATCH_label_a36; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 2: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 2: case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_76[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a37; 
                              
                              break;
                            case 1: 
                              goto MATCH_label_a1; break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_74[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a37; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_76[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a38; 
                              
                              break;
                            case 7: 
                              
                                switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                                  case 0: case 1: 
                                    MATCH_name = 
                                      MATCH_name_reg2_77[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a38; 
                                    
                                    break;
                                  case 2: 
                                    MATCH_name = 
                                      MATCH_name_reg2_61[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a38; 
                                    
                                    break;
                                  case 3: 
                                    MATCH_name = 
                                      MATCH_name_reg2_63[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a38; 
                                    
                                    break;
                                  case 4: 
                                    MATCH_name = 
                                      MATCH_name_reg2_59[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a38; 
                                    
                                    break;
                                  case 5: case 6: case 7: 
                                    goto MATCH_label_a1; break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 4: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: 
                              MATCH_name = 
                                MATCH_name_adrm_78[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a24; 
                              
                              break;
                            case 1: case 3: 
                              goto MATCH_label_a1; break;
                            case 2: case 4: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              { 
                                unsigned ea = addressToPC(MATCH_p);
                                unsigned i16 = 
                                  (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
                                
                                #line 589 "machine/mc68k/decoder_low.m"
                                 { 

                                            // HACK! Needs work

                                            int bump = 0, bumpr;

                                            RTs = instantiate (pc, "storem.l", DIS_I16,

                                                rmEA (ea, pc, bump, bumpr, 32));

                                            result.numBytes += 2; 

                                            ADDBUMP;

                                        } 

                                

                                
                                
                                
                              }
                              
                              break;
                            case 5: case 6: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              goto MATCH_label_a39; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                goto MATCH_label_a39; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 5: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: 
                              MATCH_name = 
                                MATCH_name_adrm_79[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a20; 
                              
                              break;
                            case 1: 
                              goto MATCH_label_a1; break;
                            case 2: case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_78[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a20; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_76[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a20; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_78[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a21; 
                              
                              break;
                            case 7: 
                              
                                switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                                  case 0: case 1: 
                                    MATCH_name = 
                                      MATCH_name_reg2_80[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a21; 
                                    
                                    break;
                                  case 2: case 3: case 5: case 6: case 7: 
                                    goto MATCH_label_a1; break;
                                  case 4: 
                                    MATCH_name = 
                                      MATCH_name_reg2_61[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a28; 
                                    
                                    break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 6: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 1: case 4: 
                              goto MATCH_label_a1; break;
                            case 2: case 3: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              { 
                                unsigned ea = addressToPC(MATCH_p);
                                unsigned i16 = 
                                  (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
                                
                                #line 604 "machine/mc68k/decoder_low.m"
                                 {

                                        // HACK! Requires work

                                            int bump = 0, bumpr;

                                            RTs = instantiate (pc, "loadm.l", DIS_I16,

                                                mrEA (ea, pc, bump, bumpr, 32));

                                            result.numBytes += 2; 

                                            ADDBUMP;

                                        } 

                                

                                
                                
                                
                              }
                              
                              break;
                            case 5: case 6: 
                              MATCH_w_16_16 = getWord(2 + MATCH_p); 
                              goto MATCH_label_a40; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 4) { 
                                MATCH_w_16_16 = getWord(2 + MATCH_p); 
                                goto MATCH_label_a40; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 1: case 3: case 4: 
                              goto MATCH_label_a1; break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_adrm_79[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a25; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_79[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a26; 
                              
                              break;
                            case 7: 
                              
                                switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                                  case 0: case 1: 
                                    MATCH_name = 
                                      MATCH_name_reg2_81[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a26; 
                                    
                                    break;
                                  case 2: 
                                    MATCH_name = 
                                      MATCH_name_reg2_63[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a26; 
                                    
                                    break;
                                  case 3: 
                                    MATCH_name = 
                                      MATCH_name_reg2_65[(MATCH_w_16_0 & 0x7) 
                                          /* reg2 at 0 */]; 
                                    goto MATCH_label_a26; 
                                    
                                    break;
                                  case 4: case 5: case 6: case 7: 
                                    goto MATCH_label_a1; break;
                                  default: assert(0);
                                } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          break;
        case 5: 
          
            switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
              case 0: 
                if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_86[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a44; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_60[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a44; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_84[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a44; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_82[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a44; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_86[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a45; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = 
                            MATCH_name_reg2_87[(MATCH_w_16_0 & 0x7) 
                                /* reg2 at 0 */]; 
                          goto MATCH_label_a45; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
                else 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_84[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a44; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_58[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a44; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_82[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a44; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_79[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a44; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_84[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a45; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = 
                            MATCH_name_reg2_85[(MATCH_w_16_0 & 0x7) 
                                /* reg2 at 0 */]; 
                          goto MATCH_label_a45; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
                break;
              case 1: 
                if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_90[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a46; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_64[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a46; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_88[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a46; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_86[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a46; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_90[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a47; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = 
                            MATCH_name_reg2_91[(MATCH_w_16_0 & 0x7) 
                                /* reg2 at 0 */]; 
                          goto MATCH_label_a47; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
                else 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_88[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a46; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_62[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a46; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_86[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a46; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_84[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a46; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_88[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a47; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = 
                            MATCH_name_reg2_89[(MATCH_w_16_0 & 0x7) 
                                /* reg2 at 0 */]; 
                          goto MATCH_label_a47; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
                break;
              case 2: 
                if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_94[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a48; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_68[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a48; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_92[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a48; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_90[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a48; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_94[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a49; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = 
                            MATCH_name_reg2_95[(MATCH_w_16_0 & 0x7) 
                                /* reg2 at 0 */]; 
                          goto MATCH_label_a49; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
                else 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_92[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a48; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_66[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a48; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_90[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a48; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_88[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a48; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_92[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a49; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = 
                            MATCH_name_reg2_93[(MATCH_w_16_0 & 0x7) 
                                /* reg2 at 0 */]; 
                          goto MATCH_label_a49; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
                break;
              case 3: 
                if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                  (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 2: case 3: case 4: 
                        MATCH_name = 
                          MATCH_name_cond_96[(MATCH_w_16_0 >> 8 & 0xf) 
                              /* cond at 0 */]; 
                        goto MATCH_label_a50; 
                        
                        break;
                      case 1: 
                        MATCH_w_16_16 = getWord(2 + MATCH_p); 
                        MATCH_name = 
                          MATCH_name_cond_97[(MATCH_w_16_0 >> 8 & 0xf) 
                              /* cond at 0 */]; 
                        goto MATCH_label_a51; 
                        
                        break;
                      case 5: case 6: case 7: 
                        MATCH_name = 
                          MATCH_name_cond_98[(MATCH_w_16_0 >> 8 & 0xf) 
                              /* cond at 0 */]; 
                        goto MATCH_label_a52; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
                else 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 2: case 3: case 4: 
                        MATCH_name = 
                          MATCH_name_cond_96[(MATCH_w_16_0 >> 8 & 0xf) 
                              /* cond at 0 */]; 
                        goto MATCH_label_a50; 
                        
                        break;
                      case 1: 
                        MATCH_w_16_16 = getWord(2 + MATCH_p); 
                        MATCH_name = 
                          MATCH_name_cond_97[(MATCH_w_16_0 >> 8 & 0xf) 
                              /* cond at 0 */]; 
                        goto MATCH_label_a51; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_cond_98[(MATCH_w_16_0 >> 8 & 0xf) 
                              /* cond at 0 */]; 
                        goto MATCH_label_a52; 
                        
                        break;
                      case 7: 
                        goto MATCH_label_a1; break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
                break;
              default: assert(0);
            } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
          break;
        case 6: 
          if (2 <= (MATCH_w_16_0 >> 8 & 0xf) /* cond at 0 */ && 
            (MATCH_w_16_0 >> 8 & 0xf) /* cond at 0 */ < 16) { 
            MATCH_name = 
              MATCH_name_cond_99[(MATCH_w_16_0 >> 8 & 0xf) /* cond at 0 */]; 
            { 
              char *name = MATCH_name;
              unsigned a = addressToPC(MATCH_p);
              
              #line 729 "machine/mc68k/decoder_low.m"
               { 

                      // Bcc

                          RTs = instantiate (pc, name, BTA (a, result, pc)); 

                      }

              

              
              
              
            }
            
          } /*opt-block*/
          else { 
            MATCH_name = 
              MATCH_name_cond_99[(MATCH_w_16_0 >> 8 & 0xf) /* cond at 0 */]; 
            { 
              char *name = MATCH_name;
              unsigned a = addressToPC(MATCH_p);
              
              #line 721 "machine/mc68k/decoder_low.m"
               { 

                      // _uBranch is  bra | bsr

                          strcpy(sslName, name);

                          if (strcmp (sslName, "bsr") == 0)

                              strcpy (sslName, "jsr"); 

                          RTs = instantiate (pc, sslName, BTA (a, result, pc)); 

                      }

              

              
              
              
            }
            
          } /*opt-block*/
          
          break;
        case 7: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 0) { 
            unsigned i8 = (MATCH_w_16_0 & 0xff) /* data8 at 0 */;
            unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
            
            #line 734 "machine/mc68k/decoder_low.m"
             { 

                    // moveq (semantics of move immediate long)

                        RTs = instantiate (pc, "movel", DIS_I8, DIS_DN(32)); 

                    } 

            

            
            
            
          } /*opt-block*//*opt-block+*/
          else 
            goto MATCH_label_a1;  /*opt-block+*/
          
          break;
        case 8: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_102[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a59; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_74[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a60; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_102[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_94[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_92[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_102[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a62; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = 
                            MATCH_name_reg2_103[(MATCH_w_16_0 & 0x7) 
                                /* reg2 at 0 */]; 
                          goto MATCH_label_a62; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 1: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 1: 
                        goto MATCH_label_a1; break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_104[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_102[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_94[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_104[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a64; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = 
                            MATCH_name_reg2_105[(MATCH_w_16_0 & 0x7) 
                                /* reg2 at 0 */]; 
                          goto MATCH_label_a64; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 1: 
                        goto MATCH_label_a1; break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_106[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_104[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_102[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_106[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a66; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = 
                            MATCH_name_reg2_107[(MATCH_w_16_0 & 0x7) 
                                /* reg2 at 0 */]; 
                          goto MATCH_label_a66; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_104[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a55; 
                        
                        break;
                      case 1: 
                        goto MATCH_label_a1; break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_108[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a55; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_106[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a55; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_108[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a58; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = 
                                MATCH_name_reg2_109[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a58; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_67[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a58; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_69[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a58; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_65[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a58; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          else 
            
              switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                case 0: case 2: case 3: case 4: 
                  
                    switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_sz_1[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a53; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_sz_100[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a41; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_sz_100[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a54; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_sz_5[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a55; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                  break;
                case 1: 
                  goto MATCH_label_a1; break;
                case 5: case 6: 
                  
                    switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_sz_3[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a56; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_sz_101[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a42; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_sz_101[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a57; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_sz_7[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a58; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                  break;
                case 7: 
                  if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                    (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 5) 
                    
                      switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                        case 0: 
                          MATCH_name = 
                            MATCH_name_sz_3[(MATCH_w_16_0 >> 6 & 0x3) 
                                /* sz at 0 */]; 
                          goto MATCH_label_a56; 
                          
                          break;
                        case 1: 
                          MATCH_name = 
                            MATCH_name_sz_101[(MATCH_w_16_0 >> 6 & 0x3) 
                                /* sz at 0 */]; 
                          goto MATCH_label_a42; 
                          
                          break;
                        case 2: 
                          MATCH_name = 
                            MATCH_name_sz_101[(MATCH_w_16_0 >> 6 & 0x3) 
                                /* sz at 0 */]; 
                          goto MATCH_label_a57; 
                          
                          break;
                        case 3: 
                          MATCH_name = 
                            MATCH_name_sz_7[(MATCH_w_16_0 >> 6 & 0x3) 
                                /* sz at 0 */]; 
                          goto MATCH_label_a58; 
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
                  else 
                    goto MATCH_label_a1;  /*opt-block+*/
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
          break;
        case 9: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_114[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a59; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_84[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a60; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_118[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_116[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_114[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_118[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a62; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "submb.ex"; 
                          goto MATCH_label_a62; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 1: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_116[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a75; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_86[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a76; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_120[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_118[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_116[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_120[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a64; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "submw.ex"; 
                          goto MATCH_label_a64; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_118[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a77; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_88[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a78; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_122[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_120[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_118[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_122[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a66; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "subml.ex"; 
                          goto MATCH_label_a66; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_120[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_90[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_124[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_122[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_124[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a80; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "subal.ex"; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_80[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_81[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_77[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          else 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_106[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_76[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_110[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_108[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_110[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a68; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "subrb.ex"; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_69[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_71[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_67[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 1: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_108[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_78[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_112[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_110[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_112[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a70; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "subrw.ex"; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_71[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_75[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_69[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_110[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_79[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_114[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_112[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_114[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a72; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "subrl.ex"; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_75[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_77[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_71[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_112[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_82[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_116[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_114[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_116[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a74; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "subaw.ex"; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_77[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_80[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_75[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          break;
        case 10: case 15: 
          goto MATCH_label_a1; break;
        case 11: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_130[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a81; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_106[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a60; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_134[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a81; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_132[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a81; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_134[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a82; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "eorb.ex"; 
                          goto MATCH_label_a82; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 1: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_132[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a83; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_108[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a76; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_136[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a83; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_134[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a83; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_136[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a84; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "eorw.ex"; 
                          goto MATCH_label_a84; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_134[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a85; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_110[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a78; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_138[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a85; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_136[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a85; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_138[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a86; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "eorl.ex"; 
                          goto MATCH_label_a86; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_136[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_112[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_140[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_138[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_140[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a80; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "cmpal.ex"; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_89[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_91[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_87[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          else 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_122[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_92[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_126[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_124[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_126[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a68; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "cmpb.ex"; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_81[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_83[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_80[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 1: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_124[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_94[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_128[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_126[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_128[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a70; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "cmpw.ex"; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_83[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_85[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_81[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_126[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_102[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_130[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_128[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_130[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a72; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "cmpl.ex"; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_85[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_87[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_83[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_128[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_104[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_132[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_130[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_132[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a74; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "cmpaw.ex"; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_87[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_89[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_85[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          break;
        case 12: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_138[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a59; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_114[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a60; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_144[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_140[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_138[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_144[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a62; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "andmb.ex"; 
                          goto MATCH_label_a62; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 1: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_140[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        { 
                          char *name = MATCH_name;
                          unsigned n = 
                            (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
                          unsigned n2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
                          
                          #line 970 "machine/mc68k/decoder_low.m"
                           {

                                      RTs = instantiate (pc, name, DIS_DN(32), DIS_DN2(32));

                                  }

                          

                          
                          
                          
                        }
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_116[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        { 
                          char *name = MATCH_name;
                          unsigned n = 
                            (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
                          unsigned n2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
                          
                          #line 974 "machine/mc68k/decoder_low.m"
                           {

                                      RTs = instantiate (pc, name, DIS_AN, DIS_AN2);

                                  }

                          

                          
                          
                          
                        }
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_146[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_144[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_140[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_146[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a64; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "andmw.ex"; 
                          goto MATCH_label_a64; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        goto MATCH_label_a1; break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_118[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        { 
                          char *name = MATCH_name;
                          unsigned n = 
                            (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
                          unsigned n2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
                          
                          #line 978 "machine/mc68k/decoder_low.m"
                           {

                                      RTs = instantiate (pc, name, DIS_DN(32), DIS_AN2);

                                  }

                          

                              

                          
                          
                          
                        }
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_148[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_146[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_144[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_148[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a66; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "andml.ex"; 
                          goto MATCH_label_a66; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_144[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a55; 
                        
                        break;
                      case 1: 
                        goto MATCH_label_a1; break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_150[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a55; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_148[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a55; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_146[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a55; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_150[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a58; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "muls.ex"; 
                              goto MATCH_label_a58; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_91[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a58; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_93[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a58; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_89[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a58; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          else 
            
              switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                case 0: case 2: case 3: case 4: 
                  
                    switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_sz_5[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a53; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_sz_142[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a41; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_sz_142[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a54; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_sz_12[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a55; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                  break;
                case 1: 
                  goto MATCH_label_a1; break;
                case 5: case 6: 
                  
                    switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_sz_7[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a56; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_sz_143[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a42; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_sz_143[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a57; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_sz_14[(MATCH_w_16_0 >> 6 & 0x3) 
                              /* sz at 0 */]; 
                        goto MATCH_label_a58; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                  break;
                case 7: 
                  if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                    (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 5) 
                    
                      switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                        case 0: 
                          MATCH_name = 
                            MATCH_name_sz_7[(MATCH_w_16_0 >> 6 & 0x3) 
                                /* sz at 0 */]; 
                          goto MATCH_label_a56; 
                          
                          break;
                        case 1: 
                          MATCH_name = 
                            MATCH_name_sz_143[(MATCH_w_16_0 >> 6 & 0x3) 
                                /* sz at 0 */]; 
                          goto MATCH_label_a42; 
                          
                          break;
                        case 2: 
                          MATCH_name = 
                            MATCH_name_sz_143[(MATCH_w_16_0 >> 6 & 0x3) 
                                /* sz at 0 */]; 
                          goto MATCH_label_a57; 
                          
                          break;
                        case 3: 
                          MATCH_name = 
                            MATCH_name_sz_14[(MATCH_w_16_0 >> 6 & 0x3) 
                                /* sz at 0 */]; 
                          goto MATCH_label_a58; 
                          
                          break;
                        default: assert(0);
                      } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
                  else 
                    goto MATCH_label_a1;  /*opt-block+*/
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/  
          break;
        case 13: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_154[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a59; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_128[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a60; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_160[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_158[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_156[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a61; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_160[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a62; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "addmb.ex"; 
                          goto MATCH_label_a62; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 1: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_156[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a75; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_130[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a76; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_162[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_160[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_158[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a63; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_162[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a64; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "addmw.ex"; 
                          goto MATCH_label_a64; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_158[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a77; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_132[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a78; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_164[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_162[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_160[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a65; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_164[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a66; 
                        
                        break;
                      case 7: 
                        if (0 <= (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ && 
                          (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                          MATCH_name = "addml.ex"; 
                          goto MATCH_label_a66; 
                          
                        } /*opt-block*/
                        else 
                          goto MATCH_label_a1;  /*opt-block+*/
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_160[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_134[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_166[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_164[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_162[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a79; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_166[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a80; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "addal.ex"; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_107[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_109[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_105[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a80; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          else 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_146[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_120[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_152[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_150[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_148[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a67; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_152[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a68; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "addrb.ex"; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_93[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_95[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_91[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a68; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 1: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_148[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_122[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_154[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_152[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_150[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a69; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_154[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a70; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "addrw.ex"; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_95[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_103[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_93[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a70; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_150[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_124[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_156[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_154[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_152[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a71; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_156[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a72; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "addrl.ex"; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_103[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_105[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_95[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a72; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_152[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_126[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_158[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_156[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_154[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a73; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_158[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a74; 
                        
                        break;
                      case 7: 
                        
                          switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
                            case 0: case 1: 
                              MATCH_name = "addaw.ex"; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_reg2_105[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_reg2_107[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_reg2_103[(MATCH_w_16_0 & 0x7) 
                                    /* reg2 at 0 */]; 
                              goto MATCH_label_a74; 
                              
                              break;
                            case 5: case 6: case 7: 
                              goto MATCH_label_a1; break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          break;
        case 14: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_168[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a95; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_144[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a95; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_179[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a95; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_177[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a95; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_175[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a96; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_179[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a96; 
                        
                        break;
                      case 7: 
                        MATCH_name = 
                          MATCH_name_adrm_44[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a96; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 1: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_169[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a97; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_146[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a97; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_180[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a97; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_179[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a97; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_177[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a98; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_180[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a98; 
                        
                        break;
                      case 7: 
                        MATCH_name = 
                          MATCH_name_adrm_49[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a98; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_170[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a99; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_148[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a99; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_181[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a99; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_180[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a99; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_179[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a100; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_181[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a100; 
                        
                        break;
                      case 7: 
                        MATCH_name = 
                          MATCH_name_adrm_51[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a100; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                      case 0: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 1: 
                              goto MATCH_label_a1; break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_adrm_182[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_181[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_180[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_182[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a102; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = "aslm.ex"; 
                                goto MATCH_label_a102; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 1: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 1: 
                              goto MATCH_label_a1; break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_adrm_184[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_182[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_181[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_184[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a102; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = "lslm.ex"; 
                                goto MATCH_label_a102; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 2: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 1: 
                              goto MATCH_label_a1; break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_adrm_186[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_184[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_182[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_186[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a102; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = "roxlm.ex"; 
                                goto MATCH_label_a102; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 3: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 1: 
                              goto MATCH_label_a1; break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_adrm_188[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_186[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_184[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a101; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_188[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a102; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = "rolm.ex"; 
                                goto MATCH_label_a102; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 4: case 5: case 6: case 7: 
                        goto MATCH_label_a1; break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          else 
            
              switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                case 0: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_162[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a87; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_136[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a87; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_168[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a87; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_166[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a87; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_164[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a88; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_168[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a88; 
                        
                        break;
                      case 7: 
                        MATCH_name = 
                          MATCH_name_adrm_35[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a88; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 1: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_164[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a89; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_138[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a89; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_169[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a89; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_168[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a89; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_166[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a90; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_169[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a90; 
                        
                        break;
                      case 7: 
                        MATCH_name = 
                          MATCH_name_adrm_37[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a90; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 2: 
                  
                    switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                      case 0: 
                        MATCH_name = 
                          MATCH_name_adrm_166[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a91; 
                        
                        break;
                      case 1: 
                        MATCH_name = 
                          MATCH_name_adrm_140[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a91; 
                        
                        break;
                      case 2: 
                        MATCH_name = 
                          MATCH_name_adrm_170[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a91; 
                        
                        break;
                      case 3: 
                        MATCH_name = 
                          MATCH_name_adrm_169[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a91; 
                        
                        break;
                      case 4: 
                        MATCH_name = 
                          MATCH_name_adrm_168[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a92; 
                        
                        break;
                      case 5: case 6: 
                        MATCH_name = 
                          MATCH_name_adrm_170[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a92; 
                        
                        break;
                      case 7: 
                        MATCH_name = 
                          MATCH_name_adrm_42[(MATCH_w_16_0 >> 3 & 0x7) 
                              /* adrm at 0 */]; 
                        goto MATCH_label_a92; 
                        
                        break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                  break;
                case 3: 
                  
                    switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                      case 0: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 1: 
                              goto MATCH_label_a1; break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_adrm_171[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_170[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_169[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_171[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a94; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = "asrm.ex"; 
                                goto MATCH_label_a94; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 1: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 1: 
                              goto MATCH_label_a1; break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_adrm_173[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_171[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_170[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_173[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a94; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = "lsrm.ex"; 
                                goto MATCH_label_a94; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 2: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 1: 
                              goto MATCH_label_a1; break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_adrm_175[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_173[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_171[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_175[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a94; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = "roxrm.ex"; 
                                goto MATCH_label_a94; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 3: 
                        
                          switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
                            case 0: case 1: 
                              goto MATCH_label_a1; break;
                            case 2: 
                              MATCH_name = 
                                MATCH_name_adrm_177[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 3: 
                              MATCH_name = 
                                MATCH_name_adrm_175[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 4: 
                              MATCH_name = 
                                MATCH_name_adrm_173[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a93; 
                              
                              break;
                            case 5: case 6: 
                              MATCH_name = 
                                MATCH_name_adrm_177[(MATCH_w_16_0 >> 3 & 0x7) 
                                    /* adrm at 0 */]; 
                              goto MATCH_label_a94; 
                              
                              break;
                            case 7: 
                              if (0 <= (MATCH_w_16_0 & 0x7) 
                                      /* reg2 at 0 */ && 
                                (MATCH_w_16_0 & 0x7) /* reg2 at 0 */ < 2) { 
                                MATCH_name = "rorm.ex"; 
                                goto MATCH_label_a94; 
                                
                              } /*opt-block*/
                              else 
                                goto MATCH_label_a1;  /*opt-block+*/
                              
                              break;
                            default: assert(0);
                          } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
                        break;
                      case 4: case 5: case 6: case 7: 
                        goto MATCH_label_a1; break;
                      default: assert(0);
                    } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/ 
                  break;
                default: assert(0);
              } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/  
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 12 & 0xf) -- op at 0 --*/ 
    
  }goto MATCH_finished_a; 
  
  MATCH_label_a0: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned i8 = (MATCH_w_16_16 & 0xff) /* disp8 at 16 */;
      
      #line 227 "machine/mc68k/decoder_low.m"
       {

          // _toSR (i16) [names] is privileged (we should never see it)

          // _immEAb  is  addib | andib | cmpib | eorib | orib | subib

                              int bump = 0, bumpr;

                              chop2ndLast(name);

                              RTs = instantiate(pc, sslName, DIS_I8, daEA(ea, pc,

                                  bump, bumpr, 8));

                              ADDBUMP;

                              result.numBytes += 2;

                          }

      

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a1: (void)0; /*placeholder for label*/ 
    
    #line 1123 "machine/mc68k/decoder_low.m"
       {   // the toolkit reserves "else" as a keyword, hence this code

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

    
     
    goto MATCH_finished_a; 
    
  MATCH_label_a2: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned i16 = (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      
      #line 246 "machine/mc68k/decoder_low.m"
       {

                              int bump = 0, bumpr;

                              chop2ndLast(name);

                              RTs = instantiate(pc, sslName, DIS_I16, daEA(ea, pc,

                                  bump, bumpr, 16));

                              ADDBUMP;

                              result.numBytes += 2;

                          }

      

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a3: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned i32 = 
        ((MATCH_w_16_16 & 0xffff) /* d16 at 16 */ << 16) + 
        (MATCH_w_16_32 & 0xffff) /* d16 at 32 */;
      
      #line 263 "machine/mc68k/decoder_low.m"
       {

                              int bump = 0, bumpr;

                              chop2ndLast(name);

                              RTs = instantiate(pc, sslName, DIS_I32, daEA(ea, pc,

                                  bump, bumpr, 32));

                              ADDBUMP;

                              result.numBytes += 4;

                          }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a4: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned i8 = (MATCH_w_16_16 & 0xff) /* disp8 at 16 */;
      
      #line 297 "machine/mc68k/decoder_low.m"
       {

                              int bump = 0, bumpr;

                              strcpy(sslName, name);

                              sslName[4] = '\0';         // Truncate name

                              RTs = instantiate(pc, sslName, DIS_I8, daEA(ea, pc,

                                  bump, bumpr, 8));

                              ADDBUMP;

                              result.numBytes += 2;

                          }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a5: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 280 "machine/mc68k/decoder_low.m"
       {

              // _bits    is  bchg  | bclr  | bset

              // _bitsi   is  bchgi | bclri | bseti

              // This series are assumed to be 8 bits where memory is involved,

              // or 32 for registers

                              int bump = 0, bumpr;

                              RTs = instantiate(pc, name, DIS_DN(32), daEA(ea, pc,

                                  bump, bumpr, 8));

                              ADDBUMP;

                          }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a6: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned i8 = (MATCH_w_16_16 & 0xff) /* disp8 at 16 */;
      unsigned x = 4 + addressToPC(MATCH_p);
      
      #line 239 "machine/mc68k/decoder_low.m"
       {

                              chopBoth(name);

                              RTs = instantiate(pc, sslName, DIS_I8,

                                  daEAX(eax, x, result, pc, 8));

                              result.numBytes += 2;

                          }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a7: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned i16 = (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      unsigned x = 4 + addressToPC(MATCH_p);
      
      #line 256 "machine/mc68k/decoder_low.m"
       {

                              chopBoth(name);

                              RTs = instantiate(pc, sslName, DIS_I16,

                                  daEAX(eax, x, result, pc, 16));

                              result.numBytes += 2;

                          }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a8: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned i32 = 
        ((MATCH_w_16_16 & 0xffff) /* d16 at 16 */ << 16) + 
        (MATCH_w_16_32 & 0xffff) /* d16 at 32 */;
      unsigned x = 6 + addressToPC(MATCH_p);
      
      #line 272 "machine/mc68k/decoder_low.m"
       {

                              chopBoth(name);

                              RTs = instantiate(pc, sslName, DIS_I32,

                                  daEAX(eax, x, result, pc, 32));

                              result.numBytes += 4;

                          }

      

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a9: (void)0; /*placeholder for label*/ 
    { 
      unsigned eax = addressToPC(MATCH_p);
      unsigned i8 = (MATCH_w_16_16 & 0xff) /* disp8 at 16 */;
      unsigned x = 4 + addressToPC(MATCH_p);
      
      #line 335 "machine/mc68k/decoder_low.m"
       { 

                  RTs = instantiate (pc, "btst", DIS_I8,

                      dBEAX (eax, x, result, pc, delta, 8));

                  result.numBytes += 2; 

              // MOVEP: privileged (and uncommon as well)

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a10: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned i8 = (MATCH_w_16_16 & 0xff) /* disp8 at 16 */;
      unsigned x = 4 + addressToPC(MATCH_p);
      
      #line 307 "machine/mc68k/decoder_low.m"
       {

                              strcpy(sslName, name);

                              sslName[4] = '\0';         // Truncate name

                              RTs = instantiate(pc, sslName, DIS_I8,

                                  daEAX(eax, x, result, pc, 8));

                              result.numBytes += 2;

                          }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a11: (void)0; /*placeholder for label*/ 
    { 
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 323 "machine/mc68k/decoder_low.m"
       { 

                  RTs = instantiate (pc, "btst", DIS_DN(32),

                      dBEAX (eax, x, result, pc, delta, 8));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a12: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 291 "machine/mc68k/decoder_low.m"
       {

                              chopDotex(name);

                              RTs = instantiate(pc, sslName, DIS_DN(32),

                                  daEAX(eax, x, result, pc, 8));

                          }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a13: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned i8 = (MATCH_w_16_16 & 0xff) /* disp8 at 16 */;
      
      #line 220 "machine/mc68k/decoder_low.m"
       {

          // _toCCR   is  andiToCCR | eoriToCCR | oriToCCR

                              RTs = instantiate(pc, name, DIS_I8);

                          }

      

      

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a14: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned ea2 = addressToPC(MATCH_p);
      
      #line 342 "machine/mc68k/decoder_low.m"
       { 

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

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a15: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea2 = addressToPC(MATCH_p);
      unsigned eax = addressToPC(MATCH_p);
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 383 "machine/mc68k/decoder_low.m"
       { 

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

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a16: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned d32 = 
        ((MATCH_w_16_16 & 0xffff) /* d16 at 16 */ << 16) + 
        (MATCH_w_16_32 & 0xffff) /* d16 at 32 */;
      unsigned ea2 = addressToPC(MATCH_p);
      unsigned eaxl = addressToPC(MATCH_p);
      
      #line 395 "machine/mc68k/decoder_low.m"
       { 

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

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a17: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned eax2 = addressToPC(MATCH_p);
      unsigned x2 = 2 + addressToPC(MATCH_p);
      
      #line 408 "machine/mc68k/decoder_low.m"
       { 

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

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a18: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned eax2 = addressToPC(MATCH_p);
      unsigned x = 2 + addressToPC(MATCH_p);
      unsigned x2 = 4 + addressToPC(MATCH_p);
      
      #line 421 "machine/mc68k/decoder_low.m"
       { 

                  int siz = 32;

                  if (name[4] == 'b') siz = 8;

                  if (name[4] == 'w') siz = 16;

                  strcpy(sslName, name);

                  sslName[5] = '\0';  // truncate name

                  RTs = instantiate (pc, sslName,

                      msEAX (eax, x, result, pc, delta, siz),

                      mdEAX (eax2, x2, result, pc, siz)); 

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a19: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned d32 = 
        ((MATCH_w_16_16 & 0xffff) /* d16 at 16 */ << 16) + 
        (MATCH_w_16_32 & 0xffff) /* d16 at 32 */;
      unsigned eax2 = addressToPC(MATCH_p);
      unsigned eaxl = addressToPC(MATCH_p);
      unsigned x2 = 6 + addressToPC(MATCH_p);
      
      #line 432 "machine/mc68k/decoder_low.m"
       { 

                  int siz = 32;

                  if (name[4] == 'b') siz = 8;

                  if (name[4] == 'w') siz = 16;

                  strcpy(sslName, name);

                  sslName[5] = '\0';  // truncate name

                  RTs = instantiate (pc, sslName,

                      msEAXL (eaxl, d32, result, pc, siz),

                      mdEAX (eax2, x2, result, pc, siz)); 

              }

      

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a20: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      
      #line 444 "machine/mc68k/decoder_low.m"
       { 

              // One operand instructions 

              //  _oneEAdaB is  clrb | negb | negxb | notb | tstb | nbcd | tas

              //  _oneEAdaW is  clrw | negw | negxw | notw | tstw

              //  _oneEAdaL is  clrl | negl | negxl | notl | tstl

                  int bump = 0, bumpr;

                  chopDotex(name);

                  RTs = instantiate (pc, sslName, daEA (ea, pc, bump, bumpr, 8)); 

                  ADDBUMP;

              }  

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a21: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 455 "machine/mc68k/decoder_low.m"
       { 

                  chopDotex(name);

                  RTs = instantiate (pc, sslName, daEAX (eax, x, result, pc, 8)); 

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a22: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      
      #line 460 "machine/mc68k/decoder_low.m"
       { 

                  int bump = 0, bumpr;

                  chopDotex(name);

                  RTs = instantiate (pc, sslName, daEA (ea, pc, bump, bumpr, 16)); 

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a23: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 467 "machine/mc68k/decoder_low.m"
       { 

                  chopDotex(name);

                  RTs = instantiate (pc, sslName, daEAX (eax, x, result, pc, 16)); 

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a24: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 504 "machine/mc68k/decoder_low.m"
       { 

              // _reg2only is  extw | extl | swap

              // extbl is 68020 specific

                  chopDotex(name);

                  int siz = 16;

                  if (sslName[3] == 'l') siz = 32;

                  RTs = instantiate (pc, sslName, DIS_DN(siz)); 

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a25: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      
      #line 485 "machine/mc68k/decoder_low.m"
       { 

              // _oneEAc   is  jsr | jmp | pea    

                  strcpy(sslName, name);

                  sslName[3] = '\0'; 

                  RTs = instantiate (pc, sslName, cEA (ea, pc, 32)); 

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a26: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 492 "machine/mc68k/decoder_low.m"
       { 

                  strcpy(sslName, name);

                  sslName[3] = '\0';

                  RTs = instantiate (pc, sslName,

                      cEAX (eax, x, result, pc, delta, 32)); 

              } 

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a27: (void)0; /*placeholder for label*/ 
    { 
      unsigned d4 = (MATCH_w_16_0 & 0xf) /* vect at 0 */;
      
      #line 541 "machine/mc68k/decoder_low.m"
       {

              // Trap: as far as Palm Pilots are concerned, these are NOPs, except

              // that an A-line instruction can then legally follow

                  RTs = instantiate(pc, "NOP");

                  IsTrap = true;

                  assert(d4 == d4);       // Suppress unused var warning

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a28: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      
      #line 513 "machine/mc68k/decoder_low.m"
       { 

              // _noArg is illegal | reset | nop | rte | rts | trapv | rtr

                  RTs = instantiate (pc, name); 

              // MOVE to/from SR is priveleged

              } 

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a29: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      
      #line 472 "machine/mc68k/decoder_low.m"
       { 

                  int bump = 0, bumpr;

                  chopDotex(name);

                  RTs = instantiate (pc, sslName, daEA (ea, pc, bump, bumpr, 32));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a30: (void)0; /*placeholder for label*/ 
    { 
      unsigned ea = addressToPC(MATCH_p);
      unsigned i16 = (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      
      #line 556 "machine/mc68k/decoder_low.m"
       { 

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

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a31: (void)0; /*placeholder for label*/ 
    { 
      unsigned ea = addressToPC(MATCH_p);
      unsigned i16 = (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      
      #line 574 "machine/mc68k/decoder_low.m"
       {

              // HACK! Requires work

                  int bump = 0, bumpr;

                  RTs = instantiate (pc, "loadm.w", DIS_I16,

                      mrEA (ea, pc, bump, bumpr, 16));

                  result.numBytes += 2;

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a32: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 479 "machine/mc68k/decoder_low.m"
       { 

                  chopDotex(name);

                  RTs = instantiate (pc, sslName,

                      daEAX (eax, x, result, pc, 32));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a33: (void)0; /*placeholder for label*/ 
    { 
      unsigned eax = addressToPC(MATCH_p);
      unsigned i16 = (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      unsigned x = 4 + addressToPC(MATCH_p);
      
      #line 568 "machine/mc68k/decoder_low.m"
       { 

                  RTs = instantiate (pc, "storem.w", DIS_I16, 

                      rmEAX (eax, x, result, pc, 16)); 

                  result.numBytes += 2; 

              }  

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a34: (void)0; /*placeholder for label*/ 
    { 
      unsigned eax = addressToPC(MATCH_p);
      unsigned i16 = (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      unsigned x = 4 + addressToPC(MATCH_p);
      
      #line 583 "machine/mc68k/decoder_low.m"
       {

                  RTs = instantiate (pc, "loadm.w", DIS_I16, 

                      mrEAX (eax, x, result, pc, delta, 16)); 

                  result.numBytes += 2; 

              }  

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a35: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      
      #line 531 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  RTs = instantiate (pc, name, daEA(ea, pc, bump, bumpr, 16));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a36: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 537 "machine/mc68k/decoder_low.m"
       {

                  RTs = instantiate (pc, name, daEAX(eax, x, result, pc, 16));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a37: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      
      #line 519 "machine/mc68k/decoder_low.m"
       {

              // MOVE to/from CCR

                  int bump = 0, bumpr;

                  RTs = instantiate (pc, name, dWEA(ea, pc, bump, bumpr, 16));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a38: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 526 "machine/mc68k/decoder_low.m"
       {

                  RTs = instantiate (pc, name,

                      dWEAX(eax, x, result, pc, delta, 16));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a39: (void)0; /*placeholder for label*/ 
    { 
      unsigned eax = addressToPC(MATCH_p);
      unsigned i16 = (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      unsigned x = 4 + addressToPC(MATCH_p);
      
      #line 598 "machine/mc68k/decoder_low.m"
       { 

                  RTs = instantiate (pc, "storem.l", DIS_I16, 

                      rmEAX(eax, x, result, pc, 32));

                  result.numBytes += 2; 

              }   

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a40: (void)0; /*placeholder for label*/ 
    { 
      unsigned eax = addressToPC(MATCH_p);
      unsigned i16 = (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      unsigned x = 4 + addressToPC(MATCH_p);
      
      #line 613 "machine/mc68k/decoder_low.m"
       {

                  RTs = instantiate (pc, "loadm.l", DIS_I16, 

                          mrEAX(eax, x, result, pc, delta, 32));

                  result.numBytes += 2; 

              }   

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a41: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 837 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, dEA(ea, pc, bump, bumpr, 16),

                      DIS_DN(16));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a42: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 845 "machine/mc68k/decoder_low.m"
       {

                  chopBoth(name);

                  RTs = instantiate (pc, sslName,

                      dEAX(eax, x, result, pc, delta, 16), DIS_DN(16));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a43: (void)0; /*placeholder for label*/ 
    { 
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 624 "machine/mc68k/decoder_low.m"
       { 

                  RTs = instantiate (pc, "lea",

                      cEAX (eax, x, result, pc, delta, 32), DIS_AN); 

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a44: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 629 "machine/mc68k/decoder_low.m"
       { 

              // ADDQ, SUBQ     

              // The data parameter is 3 bits (d3) but a byte will be returned anyway

                  int bump = 0, bumpr;

                  chop2ndLast(name);          // addqb -> addb

                  if (i8 == 0) i8 = 8;        // Quirk of the addq/subq instr

                  RTs = instantiate (pc, sslName, DIS_I8, alEA (ea, pc, bump, bumpr, 8)); 

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a45: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 639 "machine/mc68k/decoder_low.m"
       { 

                  chopBoth(name);             // addqb.ex -> addb

                  sslName[3] = '\0'; 

                  if (i8 == 0) i8 = 8;        // Quirk of the addq/subq instr

                  RTs = instantiate (pc, sslName, DIS_I8, alEAX (eax, x, result, pc, 8));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a46: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 646 "machine/mc68k/decoder_low.m"
       { 

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

                  if (!b)                         // Can't use else

                      chop2ndLast(name);          // addqw -> addw

                  RTs = instantiate (pc, sslName, DIS_I8, dst); 

                  ADDBUMP;

              } 

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a47: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 667 "machine/mc68k/decoder_low.m"
       { 

                  chopBoth(name);             // addqb.ex -> addb

                  if (i8 == 0) i8 = 8;        // Quirk of the addq/subq instr

                  RTs = instantiate (pc, sslName, DIS_I8,

                      alEAX (eax, x, result, pc, 16)); 

              } 

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a48: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 674 "machine/mc68k/decoder_low.m"
       { 

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

                  if (!b)                         // Can't use else

                      chop2ndLast(name);          // addqw -> addw

                  RTs = instantiate (pc, sslName, DIS_I8, dst); 

                  ADDBUMP;

              }   

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a49: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 695 "machine/mc68k/decoder_low.m"
       { 

                  chopBoth(name);             // addql.ex -> addl

                  if (i8 == 0) i8 = 8;        // Quirk of the addq/subq instr

                  RTs = instantiate (pc, sslName, DIS_I8, alEAX (eax, x, result, pc, 32));

              }   

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a50: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      
      #line 707 "machine/mc68k/decoder_low.m"
       { 

              // Scc

              // I assume that these are 8 bits where memory is involved, but

              // 32 where registers are involved

                  int bump = 0, bumpr;

                  RTs = instantiate (pc, name, daEA (ea, pc, bump, bumpr, 8));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a51: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned i16 = (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 701 "machine/mc68k/decoder_low.m"
       { 

              // DBcc     

                  RTs = instantiate (pc, name, DIS_DN(32), DIS_I16); 

                  result.numBytes += 2; 

              }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a52: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 716 "machine/mc68k/decoder_low.m"
       { 

                  RTs = instantiate (pc, name, daEAX (eax, x, result, pc, 8));

              } 

      

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a53: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 823 "machine/mc68k/decoder_low.m"
       {

              // _alurdB  is  andrb | orrb

                  int bump = 0, bumpr;

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, dEA(ea, pc, bump, bumpr, 8), DIS_DN(8));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a54: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 851 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, dEA(ea, pc, bump, bumpr, 32), DIS_DN(32));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a55: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 739 "machine/mc68k/decoder_low.m"
       { 

              // _alurdw is divs | divu | muls | mulu

              //// in order for this to match, the 'w' needs to be dropped off the

              //// SSL names DIVUw and MUL[idx]w

                  int bump = 0, bumpr;

                  RTs = instantiate (pc, name, dWEA (ea, pc, bump, bumpr, 16), DIS_DN(16)); 

                  ADDBUMP;

              }   

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a56: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 831 "machine/mc68k/decoder_low.m"
       {

                  chopBoth(name);

                  RTs = instantiate (pc, sslName,

                      dEAX(eax, x, result, pc, delta, 8), DIS_DN(8));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a57: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 858 "machine/mc68k/decoder_low.m"
       {

                  chopBoth(name);

                  RTs = instantiate (pc, sslName,

                      dEAX(eax, x, result, pc, delta, 32), DIS_DN(32));

              }

      

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a58: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 748 "machine/mc68k/decoder_low.m"
       { 

                  RTs = instantiate (pc, name,

                      dWEAX (eax, x, result, pc, delta, 16), DIS_DN(16)); 

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a59: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      unsigned n2 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 753 "machine/mc68k/decoder_low.m"
       {

              // _twoReg** (addx | subx | abcd | sbcd | cmp) 

                  strcpy(sslName, name);

                  if (strncmp(sslName, "cmp", 3) == 0) sslName[3] = '\0';

                  sslName[4] = '\0';

                  strcat(sslName, "b");         // Force 8 bit size

                  RTs = instantiate (pc, sslName, DIS_DN(8), DIS_DN2(8));

              } 

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a60: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      unsigned n2 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 762 "machine/mc68k/decoder_low.m"
       {

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

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a61: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 865 "machine/mc68k/decoder_low.m"
       {

              // _alumB   is  addmb | andmb | ormb  | submb

                  int bump = 0, bumpr;

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_DN(8), maEA(ea, pc, bump, bumpr, 8));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a62: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 873 "machine/mc68k/decoder_low.m"
       {

                  chopBoth(name);

                  RTs = instantiate (pc, sslName, DIS_DN(8),

                      maEAX(eax, x, result, pc, 8));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a63: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 879 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_DN(16), maEA(ea, pc, bump, bumpr,

                      16));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a64: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 887 "machine/mc68k/decoder_low.m"
       {

                  chopBoth(name);

                  RTs = instantiate (pc, sslName, DIS_DN(16),

                      maEAX(eax, x, result, pc, 16));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a65: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 893 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_DN(32), maEA(ea, pc, bump, bumpr,

                      32));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a66: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 901 "machine/mc68k/decoder_low.m"
       {

                  chopBoth(name);

                  RTs = instantiate (pc, sslName, DIS_DN(32),

                      maEAX(eax, x, result, pc, 32));

              }

      

      

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a67: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 983 "machine/mc68k/decoder_low.m"
       {

              // ADD, AND, CHK, CMP, CMPA, DIVS, DIVU, MULS, MULU, OR, SUB, SUBA

                  int bump = 0, bumpr;

                  chopDotex(name);           // Fix the name

                  RTs = instantiate (pc, sslName, amEA(ea, pc, bump, bumpr, 8), DIS_DN(8));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a68: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 991 "machine/mc68k/decoder_low.m"
       {

                  chopDotex(name);           // Fix the name

                  RTs = instantiate (pc, sslName,

                      amEAX(eax, x, result, pc, delta, 8), DIS_DN(8));

              }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a69: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 997 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  chopDotex(name);           // Fix the name

                  RTs = instantiate (pc, sslName, amEA(ea, pc, bump, bumpr, 16),

                      DIS_DN(16));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a70: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 1005 "machine/mc68k/decoder_low.m"
       {

                  chopDotex(name);           // Fix the name

                  RTs = instantiate (pc, sslName,

                      amEAX(eax, x, result, pc, delta, 16), DIS_DN(16));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a71: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 1011 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  chopDotex(name);           // Fix the name

                  RTs = instantiate (pc, sslName, amEA(ea, pc, bump, bumpr, 32),

                      DIS_DN(32));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a72: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 1019 "machine/mc68k/decoder_low.m"
       {

                  chopDotex(name);           // Fix the name

                  RTs = instantiate (pc, sslName,

                      amEAX(eax, x, result, pc, delta, 32), DIS_DN(32));

              }

      

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a73: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 909 "machine/mc68k/decoder_low.m"
       {

              // _aluaW   is  addaw | cmpaw | subaw 

                  int bump = 0, bumpr;

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, awlEA(ea, pc, bump, bumpr, 16), DIS_AN);

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a74: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 917 "machine/mc68k/decoder_low.m"
       {

                  chopBoth(name);

                  RTs = instantiate(pc, sslName,

                      awlEAX(eax, x, result, pc, delta, 16), DIS_AN);

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a75: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      unsigned n2 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 777 "machine/mc68k/decoder_low.m"
       {

                  strcpy(sslName, name);

                  if (strncmp(sslName, "cmp", 3) == 0) sslName[3] = '\0';

                  sslName[4] = '\0';

                  strcat(name, "w");         // Force 16 bit size

                  RTs = instantiate (pc, sslName, DIS_DN(16), DIS_DN2(16));

              } 

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a76: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      unsigned n2 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 785 "machine/mc68k/decoder_low.m"
       {

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

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a77: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      unsigned n2 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 800 "machine/mc68k/decoder_low.m"
       {

                  strcpy(sslName, name);

                  if (strncmp(sslName, "cmp", 3) == 0) sslName[3] = '\0';

                  sslName[4] = '\0';

                  strcat(sslName, "l");         // Force 32 bit size

                  RTs = instantiate (pc, sslName, DIS_DN(32), DIS_DN2(32));

              } 

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a78: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      unsigned n2 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 808 "machine/mc68k/decoder_low.m"
       {

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

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a79: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 923 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, awlEA(ea, pc, bump, bumpr, 32), DIS_AN);

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a80: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 930 "machine/mc68k/decoder_low.m"
       {

                  chopBoth(name);

                  RTs = instantiate(pc, sslName,

                      awlEAX(eax, x, result, pc, delta, 32), DIS_AN);

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a81: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 936 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  RTs = instantiate (pc, name, DIS_DN(8), daEA(ea, pc, bump, bumpr, 8));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a82: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 942 "machine/mc68k/decoder_low.m"
       {

                  RTs = instantiate (pc, name, DIS_DN(8),

                      daEAX(eax, x, result, pc, 8));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a83: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 947 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  RTs = instantiate (pc, name, DIS_DN(16), daEA(ea, pc, bump, bumpr, 16));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a84: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 953 "machine/mc68k/decoder_low.m"
       {

                  RTs = instantiate (pc, name, DIS_DN(16),

                      daEAX(eax, x, result, pc, 16));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a85: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      
      #line 958 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  RTs = instantiate (pc, name, DIS_DN(32), daEA(ea, pc, bump, bumpr, 32));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a86: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 964 "machine/mc68k/decoder_low.m"
       {

                  RTs = instantiate (pc, name, DIS_DN(32),

                      daEAX(eax, x, result, pc, 32));

              }

      

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a87: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1060 "machine/mc68k/decoder_low.m"
       {

              // _shiftIRB    is  asrib | lsrib | rorib | roxrib

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(8));

              }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a88: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1092 "machine/mc68k/decoder_low.m"
       {

              // _shiftRRB    is  asrrb | lsrrb | rorrb | roxrrb

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_DN(8), DIS_DN2(8));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a89: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1071 "machine/mc68k/decoder_low.m"
       {

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(16));

              }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a90: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1103 "machine/mc68k/decoder_low.m"
       {

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_DN(16), DIS_DN2(16));

              }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a91: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1081 "machine/mc68k/decoder_low.m"
       {

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(32));

              }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a92: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1113 "machine/mc68k/decoder_low.m"
       {

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_DN(32), DIS_DN2(32));

              }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a93: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      
      #line 1026 "machine/mc68k/decoder_low.m"
       {

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

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a94: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 1038 "machine/mc68k/decoder_low.m"
       {

                  chopDotex(name);           // Fix the name

                  RTs = instantiate (pc, sslName, maEAX(eax, x, result, pc, 16));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a95: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1066 "machine/mc68k/decoder_low.m"
       {

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(8));

              }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a96: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1098 "machine/mc68k/decoder_low.m"
       {

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_DN(8), DIS_DN2(8));

              }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a97: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1076 "machine/mc68k/decoder_low.m"
       {

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(16));

              }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a98: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1108 "machine/mc68k/decoder_low.m"
       {

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_DN(16), DIS_DN2(16));

              }

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a99: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned i8 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1086 "machine/mc68k/decoder_low.m"
       {

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_I8, DIS_DN(32));

              }

          

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a100: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned n = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
      unsigned n2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      
      #line 1118 "machine/mc68k/decoder_low.m"
       {

                  chop2ndLast(name);

                  RTs = instantiate (pc, sslName, DIS_DN(32), DIS_DN2(32));

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a101: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned ea = addressToPC(MATCH_p);
      
      #line 1043 "machine/mc68k/decoder_low.m"
       {

                  int bump = 0, bumpr;

                  chopDotex(name);           // Fix the name

                  int lst = strlen(sslName) - 1;

                  int siz = 32;

                  if (sslName[lst] == 'b') siz = 8;

                  if (sslName[lst] == 'w') siz = 16;

                  RTs = instantiate (pc, sslName, maEA(ea, pc, bump, bumpr, siz));

                  ADDBUMP;

              }

      

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_label_a102: (void)0; /*placeholder for label*/ 
    { 
      char *name = MATCH_name;
      unsigned eax = addressToPC(MATCH_p);
      unsigned x = 2 + addressToPC(MATCH_p);
      
      #line 1054 "machine/mc68k/decoder_low.m"
       {

                  chopDotex(name);           // Fix the name

                  RTs = instantiate (pc, sslName, maEAX(eax, x, result, pc, 16));

              }

      

          

      
      
      
    } 
    goto MATCH_finished_a; 
    
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}

#line 1135 "machine/mc68k/decoder_low.m"

    result.numBytes += 2;           // Count the main opcode
    return RTs;
}




