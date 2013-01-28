/*
 * Copyright (C) 2000-2001, The University of Queensland
 * Copyright (C) 2000, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

#define sign_extend(N,SIZE) (((int)((N) << (sizeof(unsigned)*8-(SIZE)))) >> (sizeof(unsigned)*8-(SIZE)))
#include <assert.h>


/*==============================================================================
 * FILE:       decoder.m
 * OVERVIEW:   Implementation of the higher level mc68000 specific parts of the
 *             NJMCDecoder class.
 *============================================================================*/

/* $Revision$
 * $Id$
 * Created by Mike 15 Feb 2000
 * 21 Feb 2000 - Mike: support for -(an) and (an)+ (bump and bumpr)
 * 21 Mar 2000 - Mike: pPcDisp generates expressions with idCodeAddr
 * 24 Mar 2000 - Mike: Converted sizes to bits; initialise size of SemStr
 *  1 Aug 00 - Cristina: upgraded to support mltk version Apr 5 14:56:28
 *              EDT 2000.  [numBytes] renamed to nextPC as semantics of
 *              [numBytes] has changed.  Assignment of (nextPC - hostPC). 
*/

#include "global.h"
#include "decoder.h"
#include "prog.h"
#include "ss.h"
#include "rtl.h"
#include "proc.h"
#include "csr.h"
#include "mc68k.pat.h"

/**********************************
 * NJMCDecoder methods.
 **********************************/   

/*==============================================================================
 * FUNCTION:       NJMCDecoder::decodeInstruction
 * OVERVIEW:       Decodes a machine instruction and returns an RTL instance. In
 *                 most cases a single instruction is decoded. However, if a
 *                 higher level construct that may consist of multiple
 *                 instructions is matched, then there may be a need to return
 *                 more than one RTL. The caller_prologue2 is an example of such
 *                 a construct which encloses an abritary instruction that must
 *                 be decoded into its own RTL (386 example)
 * PARAMETERS:     pc - the native address of the pc
 *                 delta - the difference between the above address and the
 *                   host address of the pc (i.e. the address that the pc is at
 *                   in the loaded object file)
 *                 RTLDict - the dictionary of RTL templates used to instantiate
 *                   the RTL for the instruction being decoded
 *                 proc - the enclosing procedure
 *                 pProc - the enclosing procedure
 * \returns         a DecodeResult structure containing all the information
 *                   gathered during decoding
 *============================================================================*/
DecodeResult& NJMCDecoder::decodeInstruction (ADDRESS pc, int delta,
    UserProc* proc = NULL)
{
    static DecodeResult result;
    ADDRESS hostPC = pc + delta;

    // Clear the result structure;
    result.reset();

    // The actual list of instantiated RTs
    list<RT*>* RTs = NULL;

    // Try matching a logue first
    int addr, regs, locals, stackSize, d16, d32, reg;
    ADDRESS saveHostPC = hostPC;
    Logue* logue;
    if ((logue = InstructionPatterns::std_call(csr, hostPC, addr)) != NULL) {
        /*
         * Direct call
         */
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.numBytes = hostPC - saveHostPC;

        // Set the destination expression
        newCall->setDest(addr - delta);
        newCall->setPrologue(logue);

        // Save RTL for the latest call
        //lastCall = newCall;
        SHOW_ASM("std_call "<<addr)
    }

    else if ((logue = InstructionPatterns::near_call(csr, hostPC, addr))
        != NULL) {
        /*
         * Call with short displacement (16 bit instruction)
         */
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.numBytes = hostPC - saveHostPC;

        // Set the destination expression
        newCall->setDest(addr - delta);
        newCall->setPrologue(logue);

        // Save RTL for the latest call
        //lastCall = newCall;
        SHOW_ASM("near_call " << addr)
    }

    else if ((logue = InstructionPatterns::pea_pea_add_rts(csr, hostPC, d32))
        != NULL) {
        /*
         * pea E(pc) pea 4(pc) / addil #d32, (a7) / rts
         * Handle as a call
         */
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.numBytes = hostPC - saveHostPC;

        // Set the destination expression. It's d32 past the address of the
        // d32 itself, which is pc+10
        newCall->setDest(pc + 10 + d32);
        newCall->setPrologue(logue);

        // Save RTL for the latest call
        //lastCall = newCall;
        SHOW_ASM("pea/pea/add/rts " << pc+10+d32)
    }

    else if ((logue = InstructionPatterns::pea_add_rts(csr, hostPC, d32))
        != NULL) {
        /*
         * pea 4(pc) / addil #d32, (a7) / rts
         * Handle as a call followed by a return
         */
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.numBytes = hostPC - saveHostPC;

        // Set the destination expression. It's d32 past the address of the
        // d32 itself, which is pc+6
        newCall->setDest(pc + 6 + d32);
        newCall->setPrologue(logue);

        // This call effectively is followed by a return
        newCall->setReturnAfterCall(true);

        // Save RTL for the latest call
        //lastCall = newCall;
        SHOW_ASM("pea/add/rts " << pc+6+d32)
    }

    else if ((logue = InstructionPatterns::trap_syscall(csr, hostPC, d16))
        != NULL) {
        /*
         * trap / AXXX  (d16 set to the XXX)
         * Handle as a library call
         */
        HLCall* newCall = new HLCall(pc, 0, RTs);
        result.rtl = newCall;
        result.numBytes = hostPC - saveHostPC;

        // Set the destination expression. For now, we put AAAAA000+d16 there
        newCall->setDest(0xAAAAA000 + d16);
        newCall->setPrologue(logue);

        SHOW_ASM("trap/syscall " << hex << 0xA000 + d16)
    }

/*
 * CALLEE PROLOGUES
 */
    else if ((logue = InstructionPatterns::link_save(csr, hostPC,
        locals, d16)) != NULL)
    {
        /*
         * Standard link with save of registers using movem
         */
        if (proc != NULL) {

            // Record the prologue of this callee
            assert(logue->getType() == Logue::CALLEE_PROLOGUE);
            proc->setPrologue((CalleePrologue*)logue);
        }
        result.rtl = new RTlist(pc, RTs);
        result.numBytes = hostPC - saveHostPC;
        SHOW_ASM("link_save " << locals)
    }

    else if ((logue = InstructionPatterns::link_save1(csr, hostPC,
        locals, reg)) != NULL)
    {
        /*
         * Standard link with save of 1 D register using move dn,-(a7)
         */
        if (proc != NULL) {

            // Record the prologue of this callee
            assert(logue->getType() == Logue::CALLEE_PROLOGUE);
            proc->setPrologue((CalleePrologue*)logue);
        }
        result.rtl = new RTlist(pc, RTs);
        result.numBytes = hostPC - saveHostPC;
        SHOW_ASM("link_save1 " << locals)
    }

    else if ((logue = InstructionPatterns::push_lea(csr, hostPC,
        locals, reg)) != NULL)
    {
        /*
         * Just save of 1 D register using move dn,-(a7);
         * then an lea d16(a7), a7 to allocate the stack
         */
        //locals = 0;             // No locals for this prologue
        if (proc != NULL) {

            // Record the prologue of this callee
            assert(logue->getType() == Logue::CALLEE_PROLOGUE);
            proc->setPrologue((CalleePrologue*)logue);
        }
        result.rtl = new RTlist(pc, RTs);
        result.numBytes = hostPC - saveHostPC;
        SHOW_ASM("push_lea " << locals)
    }

    else if ((logue = InstructionPatterns::std_link(csr, hostPC,
        locals)) != NULL)
    {
        /*
         * Standard link
         */
        if (proc != NULL) {

            // Record the prologue of this callee
            assert(logue->getType() == Logue::CALLEE_PROLOGUE);
            proc->setPrologue((CalleePrologue*)logue);
        }
        result.rtl = new RTlist(pc, RTs);
        result.numBytes = hostPC - saveHostPC;
        SHOW_ASM("std_link "<< locals)
    }

    else if ((logue = InstructionPatterns::bare_ret(csr, hostPC))
        != NULL)
    {
        /*
         * Just a bare rts instruction
         */
        if (proc != NULL) {

            // Record the prologue of this callee
            assert(logue->getType() == Logue::CALLEE_PROLOGUE);
            proc->setPrologue((CalleePrologue*)logue);
            proc->setEpilogue(new CalleeEpilogue("__dummy",list<string>()));
        }
        result.rtl = new HLReturn(pc, RTs);
        result.numBytes = hostPC - saveHostPC;
        SHOW_ASM("bare_ret")
    }

    else if ((logue = InstructionPatterns::std_ret(csr, hostPC)) != NULL) {
        /*
         * An unlink and return
         */
        if (proc!= NULL) {

            // Record the epilogue of this callee
            assert(logue->getType() == Logue::CALLEE_EPILOGUE);
            proc->setEpilogue((CalleeEpilogue*)logue);
        }

        result.rtl = new HLReturn(pc, RTs);
        result.numBytes = hostPC - saveHostPC;
        SHOW_ASM("std_ret")
    }

    else if ((logue = InstructionPatterns::rest_ret(csr, hostPC, d16)) != NULL)
    {
        /*
         * A restore (movem stack to registers) then return
         */
        if (proc!= NULL) {

            // Record the epilogue of this callee
            assert(logue->getType() == Logue::CALLEE_EPILOGUE);
            proc->setEpilogue((CalleeEpilogue*)logue);
        }

        result.rtl = new HLReturn(pc, RTs);
        result.numBytes = hostPC - saveHostPC;
        SHOW_ASM("rest_ret")
    }

    else if ((logue = InstructionPatterns::rest1_ret(csr, hostPC, reg)) != NULL)
    {
        /*
         * A pop (move (a7)+ to one D register) then unlink and return
         */
        if (proc!= NULL) {

            // Record the epilogue of this callee
            assert(logue->getType() == Logue::CALLEE_EPILOGUE);
            proc->setEpilogue((CalleeEpilogue*)logue);
        }

        result.rtl = new HLReturn(pc, RTs);
        result.numBytes = hostPC - saveHostPC;
        SHOW_ASM("rest1_ret")
    }

    else if ((logue = InstructionPatterns::pop_ret(csr, hostPC, reg)) != NULL)
    {
        /*
         * A pop (move (a7)+ to one D register) then just return
         */
        if (proc!= NULL) {

            // Record the epilogue of this callee
            assert(logue->getType() == Logue::CALLEE_EPILOGUE);
            proc->setEpilogue((CalleeEpilogue*)logue);
        }

        result.rtl = new HLReturn(pc, RTs);
        result.numBytes = hostPC - saveHostPC;
        SHOW_ASM("pop_ret")
    }

    else if ((logue = InstructionPatterns::clear_stack(csr, hostPC, stackSize))
        != NULL)
    {
        /*
         * Remove parameters from the stack
         */
        RTs = instantiate(pc, "clear_stack", dis_Num(stackSize));

        result.rtl = new RTlist(pc, RTs);
        result.numBytes = hostPC - saveHostPC;
    }

    else {

        ADDRESS nextPC;
        int bump = 0, bumpr;
        SemStr* ss;




{ 
  dword MATCH_p = 
    

    hostPC
    ;
  char *MATCH_name;
  static char *MATCH_name_cond_12[] = {
    "bra", (char *)0, "bhi", "bls", "bcc", "bcs", "bne", "beq", "bvc", "bvs", 
    "bpl", "bmi", "bge", "blt", "bgt", "ble", 
  };
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */) {
        case 0: case 1: case 2: case 3: case 7: case 8: case 9: case 10: 
        case 11: case 12: case 13: case 14: case 15: 
          goto MATCH_label_de0; break;
        case 4: 
          if ((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 2) 
            if ((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */ == 7) 
              if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
                goto MATCH_label_de0;  /*opt-block+*/
              else 
                
                  switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                    case 0: case 1: 
                      goto MATCH_label_de0; break;
                    case 2: 
                      { 
                        unsigned ea = addressToPC(MATCH_p);
                        nextPC = 2 + MATCH_p; 
                        

                        

                                    /*

                                     * Register call

                                     */

                                    // Mike: there should probably be a HLNwayCall class for this!

                                    HLCall* newCall = new HLCall(pc, 0, RTs);

                                    // Record the fact that this is a computed call

                                    newCall->setIsComputed();

                                    // Set the destination expression

                                    newCall->setDest(cEA(ea, pc, 32));

                                    result.rtl = newCall;

                                    // Only one instruction, so size of result is size of this decode

                                    result.numBytes = nextPC - hostPC;

                            

                        
                        
                        
                      }
                      
                      break;
                    case 3: 
                      { 
                        unsigned ea = addressToPC(MATCH_p);
                        nextPC = 2 + MATCH_p; 
                        

                        

                                    /*

                                     * Register jump

                                     */

                                    HLNwayJump* newJump = new HLNwayJump(pc, RTs);

                                    // Record the fact that this is a computed call

                                    newJump->setIsComputed();

                                    // Set the destination expression

                                    newJump->setDest(cEA(ea, pc, 32));

                                    result.rtl = newJump;

                                    // Only one instruction, so size of result is size of this decode

                                    result.numBytes = nextPC - hostPC;

                                

                                /*

                                 * Unconditional branches

                                 */

                        
                        
                        
                      }
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/   
            else 
              goto MATCH_label_de0;  /*opt-block+*/ 
          else 
            goto MATCH_label_de0;  /*opt-block+*/
          break;
        case 5: 
          
            switch((MATCH_w_16_0 >> 8 & 0xf) /* cond at 0 */) {
              case 0: case 1: case 8: case 9: 
                goto MATCH_label_de0; break;
              case 2: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "shi"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JSG)

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              case 3: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "sls"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JULE)

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              case 4: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "scc"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JUGE)

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              case 5: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "scs"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JUL)

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              case 6: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "sne"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JNE)

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              case 7: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "seq"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JE)

                            //| svc(ea) [name] =>

                            //  ss = daEA(ea, pc, bump, bumpr, 1);

                            //    RTs = instantiate(pc, name, ss);

                            //    SETS(name, ss, HLJCOND_)

                            //| svs(ea) [name] =>

                            //  ss = daEA(ea, pc, bump, bumpr, 1);

                            //    RTs = instantiate(pc, name, ss);

                            //    SETS(name, ss, HLJCOND_)

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              case 10: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "spl"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JPOS)

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              case 11: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "smi"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JMI)

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              case 12: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "sge"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JSGE)

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              case 13: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "slt"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JSL)

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              case 14: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "sgt"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JSG)

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              case 15: 
                if (((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 0 || 
                  2 <= (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ && 
                  (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ < 5) && 
                  (MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) { 
                  MATCH_name = "sle"; 
                  { 
                    char *name = MATCH_name;
                    unsigned ea = addressToPC(MATCH_p);
                    nextPC = 2 + MATCH_p; 
                    

                    

                                ss = daEA(ea, pc, bump, bumpr, 1);

                                RTs = instantiate(pc, name, ss);

                                SETS(name, ss, HLJCOND_JSLE)

                    // HACK: Still need to do .ex versions of set, jsr, jmp

                        

                    
                    
                    
                  }
                  
                } /*opt-block*/
                else 
                  goto MATCH_label_de0;  /*opt-block+*/
                
                break;
              default: assert(0);
            } /* (MATCH_w_16_0 >> 8 & 0xf) -- cond at 0 --*/ 
          break;
        case 6: 
          
            switch((MATCH_w_16_0 >> 8 & 0xf) /* cond at 0 */) {
              case 0: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              UNCOND_JUMP(name, nextPC - hostPC, ss);

                      

                          /*

                           * Conditional branches

                           */

                  
                  
                  
                }
                
                break;
              case 1: 
                goto MATCH_label_de0; break;
              case 2: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JUG)

                  
                  
                  
                }
                
                break;
              case 3: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JULE)

                  
                  
                  
                }
                
                break;
              case 4: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JUGE)

                  
                  
                  
                }
                
                break;
              case 5: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JUL)

                  
                  
                  
                }
                
                break;
              case 6: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JNE)

                  
                  
                  
                }
                
                break;
              case 7: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JE)

                  
                  
                  
                }
                
                break;
              case 8: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, (JCOND_TYPE)0)

                  
                  
                  
                }
                
                break;
              case 9: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, (JCOND_TYPE)0)

                      

                      

                          // MVE: I'm assuming that we won't ever see shi(-(a7)) or the like.

                          // This would unbalance the stack, although it would be legal for

                          // address registers other than a7. For now, we ignore the possibility

                          // of having to bump a register

                  
                  
                  
                }
                
                break;
              case 10: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JPOS)

                  
                  
                  
                }
                
                break;
              case 11: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JMI)

                  
                  
                  
                }
                
                break;
              case 12: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JSGE)

                  
                  
                  
                }
                
                break;
              case 13: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JSL)

                  
                  
                  
                }
                
                break;
              case 14: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JSG)

                  
                  
                  
                }
                
                break;
              case 15: 
                MATCH_name = MATCH_name_cond_12[(MATCH_w_16_0 >> 8 & 0xf) 
                      /* cond at 0 */]; 
                { 
                  char *name = MATCH_name;
                  unsigned d = addressToPC(MATCH_p);
                  nextPC = 2 + MATCH_p; 
                  

                  

                              ss = BTA(d, result, pc);

                              COND_JUMP(name, nextPC - hostPC, ss, HLJCOND_JSLE)

                  
                  
                  
                }
                
                break;
              default: assert(0);
            } /* (MATCH_w_16_0 >> 8 & 0xf) -- cond at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 12 & 0xf) -- op at 0 --*/ 
    
  }goto MATCH_finished_de; 
  
  MATCH_label_de0: (void)0; /*placeholder for label*/ 
    { 
      nextPC = MATCH_p; 
      

      
                  result.rtl = new RTlist(pc,

                      decodeLowLevelInstruction(hostPC, pc, result));

      
      
      
    } 
    goto MATCH_finished_de; 
    
  MATCH_finished_de: (void)0; /*placeholder for label*/
  
}


    }
    return result;
}

/*==============================================================================
 * These are machine specific functions used to decode instruction
 * operands into SemStrs.
 *============================================================================*/

/*
 * Modified from the original: 
 *  m68k_ea.m
 *  written by Owen Braun
 *  ocbraun@princeton.edu
 *  created 4/7/96 11:27 pm
 * 
 * 4 Feb 2000 - Cristina, removed ObjFile reference for integration w/UQBT 
 * 7,9 Feb 2000 - Cristina, changed display format of addressing modes to
 *      comply with the Motorola assembly format.
 * 8 Feb 2000 - Note that the ML version of the toolkit generates
 *      repeated label names for procedures that have more than one
 *      matching statement.  Mike's program formats replaces repeated
 *      names by unique names.
 * 9 Feb 00 - Cristina.  Note that I have *not* tested the indexed 
 *        addressing modes as the mc68328 does not support *any* of
 *        such addressing modes.  The mc68000 does however. 
 * 20 Feb 00 - Cristina: fixed branches
 * 21 Feb 00 - Mike: Removed redundant delta from BTA()
 * 01 Aug 01 - Mike: Quelled some warnings about "name" not used
 */


// Branch target
SemStr* NJMCDecoder::BTA(ADDRESS d, DecodeResult& result, ADDRESS pc)
{

  SemStr* ret = new SemStr(32);
  ret->push(idIntConst);




{ 
  dword MATCH_p = 
    

    d
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 & 0xff) /* data8 at 0 */ == 0) { 
      MATCH_w_16_16 = getWord(2 + MATCH_p); 
      { 
        int /* [~32768..32767] */ dsp16 = 
          sign_extend((MATCH_w_16_16 & 0xffff) /* d16 at 16 */, 16);
        

         {

                        ret->push(pc+2 + dsp16);

                        result.numBytes += 2;

                    }

        

        
        
        
      }
      
    } /*opt-block*/
    else { 
      unsigned dsp8 = (MATCH_w_16_0 & 0xff) /* data8 at 0 */;
      

        {

                      // Casts needed to work around MLTK bug

                      ret->push(pc+2 + (int)(char)dsp8);

                  }

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_ce; 
  
  MATCH_finished_ce: (void)0; /*placeholder for label*/
  
}


    
  return ret;
}


void NJMCDecoder::pIllegalMode(ADDRESS pc)
{
    ostrstream ost;
    ost << "Illegal addressing mode at " << hex << pc;
    error(str(ost));
}

SemStr* NJMCDecoder::pDDirect(int r2, int size)
{
    SemStr* ret = new SemStr(size);
    ret->push(idRegOf);
    ret->push(idIntConst);
    ret->push(r2);
    return ret;
}

SemStr* NJMCDecoder::pADirect(int r2, int size)
{
    SemStr* ret = new SemStr(size);
    ret->push(idRegOf);
    ret->push(idIntConst);
    ret->push(r2 + 8);          // First A register is r[8]
    return ret;
}

SemStr* NJMCDecoder::pIndirect(int r2, int size)
{
    SemStr* ret = new SemStr(size);
    ret->push(idMemOf);
    ret->push(idRegOf);
    ret->push(idIntConst);
    ret->push(r2 + 8);          // First A register is r[8]
    return ret;
}

SemStr* NJMCDecoder::pPostInc(int r2, int& bump, int& bumpr, int size)
{
    // Treat this as (an), set bump to size, and bumpr to r2+8
    // Note special semantics when r2 == 7 (stack pointer): if size == 1, then
    // the system will change it to 2 to keep the stack word aligned
    if ((r2 == 7) && (size == 8)) size = 16;
    SemStr* ret = new SemStr(size/8);
    ret->push(idMemOf);
    ret->push(idRegOf);
    ret->push(idIntConst);
    ret->push(r2 + 8);          // First A register is r[8]
    bump = size/8;              // Amount to bump register by
    bumpr = r2 + 8;             // Register to bump
    return ret;
}

SemStr* NJMCDecoder::pPreDec(int r2, int& bump, int& bumpr, int size)
{
    // We treat this as -size(an), set bump to -size, and bumpr to r2+8
    // Note special semantics when r2 == 7 (stack pointer): if size == 1, then
    // the system will change it to 2 to keep the stack word aligned
    if ((r2 == 7) && (size == 8)) size = 16;
    SemStr* ret = new SemStr(size);
    ret->push(idMemOf); ret->push(idPlus);
    ret->push(idRegOf);
    ret->push(idIntConst);
    ret->push(r2 + 8);          // First A register is r[8]
    ret->push(idIntConst); ret->push(-size/8);
    bump = -size/8;             // Amount to bump register by
    bumpr = r2 + 8;             // Register to bump
    return ret;
}

SemStr* NJMCDecoder::alEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pDDirect(reg2, size);

            
            
            
          }
          
          break;
        case 1: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pADirect(reg2, size);

            
            
            
          }
          
          break;
        case 2: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pIndirect(reg2, size);

            
            
            
          }
          
          break;
        case 3: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPostInc(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 4: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPreDec(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_be; 
  
  MATCH_finished_be: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::amEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pDDirect(reg2, size);

            
            
            
          }
          
          break;
        case 1: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pADirect(reg2, size);

            
            
            
          }
          
          break;
        case 2: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pIndirect(reg2, size);

            
            
            
          }
          
          break;
        case 3: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPostInc(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 4: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPreDec(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_ae; 
  
  MATCH_finished_ae: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::awlEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pDDirect(reg2, size);

            
            
            
          }
          
          break;
        case 1: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pADirect(reg2, size);

            
            
            
          }
          
          break;
        case 2: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pIndirect(reg2, size);

            
            
            
          }
          
          break;
        case 3: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPostInc(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 4: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPreDec(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_zd; 
  
  MATCH_finished_zd: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::cEA(ADDRESS ea, ADDRESS pc, int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 2) { 
      unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
      

       ret = pIndirect(reg2, size);

      
      
      
    } /*opt-block*//*opt-block+*/
    else 
      

      pIllegalMode(pc);

      
       /*opt-block+*/
    
  }goto MATCH_finished_yd; 
  
  MATCH_finished_yd: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::dEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pDDirect(reg2, size);

            
            
            
          }
          
          break;
        case 1: case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        case 2: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pIndirect(reg2, size);

            
            
            
          }
          
          break;
        case 3: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPostInc(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 4: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPreDec(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_xd; 
  
  MATCH_finished_xd: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::daEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pDDirect(reg2, size);

            
            
            
          }
          
          break;
        case 1: case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        case 2: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pIndirect(reg2, size);

            
            
            
          }
          
          break;
        case 3: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPostInc(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 4: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPreDec(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_wd; 
  
  MATCH_finished_wd: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::dBEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pDDirect(reg2, size);

            
            
            
          }
          
          break;
        case 1: case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        case 2: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pIndirect(reg2, size);

            
            
            
          }
          
          break;
        case 3: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPostInc(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 4: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPreDec(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_vd; 
  
  MATCH_finished_vd: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::dWEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pDDirect(reg2, size);

            
            
            
          }
          
          break;
        case 1: case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        case 2: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pIndirect(reg2, size);

            
            
            
          }
          
          break;
        case 3: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPostInc(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 4: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPreDec(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_ud; 
  
  MATCH_finished_ud: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::maEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        case 2: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pIndirect(reg2, size);

            
            
            
          }
          
          break;
        case 3: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPostInc(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 4: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPreDec(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_td; 
  
  MATCH_finished_td: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::msEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pDDirect(reg2, size);

            
            
            
          }
          
          break;
        case 1: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pADirect(reg2, size);

            
            
            
          }
          
          break;
        case 2: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pIndirect(reg2, size);

            
            
            
          }
          
          break;
        case 3: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPostInc(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 4: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPreDec(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_sd; 
  
  MATCH_finished_sd: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::mdEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */) {
        case 0: 
          { 
            unsigned reg1 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
            

             ret = pDDirect(reg1, size);

            
            
            
          }
          
          break;
        case 1: 
          { 
            unsigned reg1 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
            

             ret = pADirect(reg1, size);

            
            
            
          }
          
          break;
        case 2: 
          { 
            unsigned reg1 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
            

             ret = pIndirect(reg1, size);

            
            
            
          }
          
          break;
        case 3: 
          { 
            unsigned reg1 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
            

             ret = pPostInc(reg1, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 4: 
          { 
            unsigned reg1 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
            

             ret = pPreDec(reg1, bump, bumpr, size);

            
            
            
          }
          
          break;
        case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 6 & 0x7) -- MDadrm at 0 --*/ 
    
  }goto MATCH_finished_rd; 
  
  MATCH_finished_rd: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::mrEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 4: case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        case 2: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pIndirect(reg2, size);

            
            
            
          }
          
          break;
        case 3: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPostInc(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_qd; 
  
  MATCH_finished_qd: (void)0; /*placeholder for label*/
  
}


  return ret;
}

SemStr* NJMCDecoder::rmEA(ADDRESS ea, ADDRESS pc, int& bump, int& bumpr,
    int size)
{
  SemStr* ret = new SemStr(size);



{ 
  dword MATCH_p = 
    

    ea
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 3: case 5: case 6: case 7: 
          

          pIllegalMode(pc);

          
          
          
          break;
        case 2: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pIndirect(reg2, size);

            
            
            
          }
          
          break;
        case 4: 
          { 
            unsigned reg2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             ret = pPreDec(reg2, bump, bumpr, size);

            
            
            
          }
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_pd; 
  
  MATCH_finished_pd: (void)0; /*placeholder for label*/
  
}


  return ret;
}


SemStr* NJMCDecoder::pADisp(int d16, int r, int size)
{
  SemStr* ret = new SemStr(size);
  // d16(Ar) -> m[ + r[ int r+8 ] int d16]
  ret->push(idMemOf); ret->push(idPlus); ret->push(idRegOf);
  ret->push(idIntConst); ret->push(r+8);
  ret->push(idIntConst); ret->push(d16);
  return ret;
}

SemStr* NJMCDecoder::pAIndex(int d8, int r, int iT, int iR, int iS, int size)
{
    SemStr* ret = new SemStr(size);
    // d8(Ar, A/Di.[wl] ->
    //   m[ + + r[ int r+8 ] ! size[ 16/32 r[ int iT<<3+iR ]] int i8 ]
    ret->push(idMemOf); ret->push(idPlus); ret->push(idPlus);
    ret->push(idRegOf); ret->push(idIntConst); ret->push(r+8);
    ret->push(idSignExt);
    ret->push(idSize);  ret->push(iS == 0 ? 16 : 32);
    ret->push(idRegOf); ret->push(idIntConst); ret->push((iT<<3) + iR);
    ret->push(idIntConst); ret->push(d8);
    return ret;
}

SemStr* NJMCDecoder::pPcDisp(ADDRESS label, int delta, int size)
{
    // Note: label is in the host address space, so need to subtract delta
    SemStr* ret = new SemStr(size);
    // d16(pc) -> m[ code pc+d16 ]
    // Note: we use "code" instead of "int" to flag the fact that this address
    // is relative to the pc, and hence needs translation before use in the
    // target machine
    ret->push(idMemOf); ret->push(idCodeAddr);
    ret->push(label - delta);
    return ret;
}

SemStr* NJMCDecoder::pPcIndex(int d8, int iT, int iR, int iS, ADDRESS nextPc, int size)
{
    // Note: nextPc is expected to have +2 or +4 etc already added to it!
    SemStr* ret = new SemStr(size);
    // d8(pc, A/Di.[wl] ->
    //   m[ + pc+i8 ! size[ 16/32 r[ int iT<<3+iR ]]]
    ret->push(idMemOf); ret->push(idPlus);
    ret->push(idIntConst); ret->push(nextPc+d8);
    ret->push(idSignExt);
    ret->push(idSize);  ret->push(iS == 0 ? 16 : 32);
    ret->push(idRegOf); ret->push(idIntConst); ret->push((iT<<3) + iR);
    return ret;
}

SemStr* NJMCDecoder::pAbsW(int d16, int size)
{
  // (d16).w  ->  size[ ss m[ int d16 ]]
  // Note: d16 should already have been sign extended to 32 bits
  SemStr* ret = new SemStr(size);
  ret->push(idSize); ret->push(size);
  ret->push(idMemOf); ret->push(idIntConst); ret->push(d16);
  return ret;
}

SemStr* NJMCDecoder::pAbsL(int d32, int size)
{
  // (d32).w  ->  size[ ss m[ int d32 ]]
  SemStr* ret = new SemStr(size);
  ret->push(idSize); ret->push(size);
  ret->push(idMemOf); ret->push(idIntConst); ret->push(d32);
  return ret;
}

SemStr* NJMCDecoder::pImmB(int d8)
{
  // #d8 -> int d8
  // Should already be sign extended to 32 bits
  SemStr* ret = new SemStr(8);
  ret->push(idIntConst); ret->push(d8);
  return ret;
}

SemStr* NJMCDecoder::pImmW(int d16)
{
  // #d16 -> int d16
  // Should already be sign extended to 32 bits
  SemStr* ret = new SemStr(16);
  ret->push(idIntConst); ret->push(d16);
  return ret;
}

SemStr* NJMCDecoder::pImmL(int d32)
{
  SemStr* ret = new SemStr(32);
  ret->push(idIntConst); ret->push(d32);
  return ret;
}

void NJMCDecoder::pNonzeroByte(ADDRESS pc)
{
    ostrstream ost;
    ost << "Non zero upper byte at " << hex << pc;
    error(str(ost));
}


SemStr* NJMCDecoder::alEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_od0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: 
                

                 { mode = 5; result.numBytes += 4;}

                
                
                
                break;
              case 2: case 3: case 4: case 5: case 6: case 7: 
                goto MATCH_label_od0; break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_od; 
  
  MATCH_label_od0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_od; 
    
  MATCH_finished_od: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_nd; 
  
  MATCH_finished_nd: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_md; 
  
  MATCH_finished_md: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_ld; 
  
  MATCH_finished_ld: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_kd; 
  
  MATCH_finished_kd: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::amEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_jd0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: 
                

                 { mode = 5; result.numBytes += 4;}

                
                
                
                break;
              case 2: 
                

                 { mode = 2; result.numBytes += 2;}

                
                
                
                break;
              case 3: 
                

                 { mode = 3; result.numBytes += 2;}

                
                
                
                break;
              case 4: 
                
                  switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                    case 0: 
                      

                       { mode = 6; result.numBytes += 2;}

                      
                      
                      
                      break;
                    case 1: 
                      

                       { mode = 7; result.numBytes += 2;}

                      
                      
                      
                      break;
                    case 2: 
                      

                       { mode = 8; result.numBytes += 4;}

                      
                      
                      
                      break;
                    case 3: 
                      goto MATCH_label_jd0; break;
                    default: assert(0);
                  } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                break;
              case 5: case 6: case 7: 
                goto MATCH_label_jd0; break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_jd; 
  
  MATCH_label_jd0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_jd; 
    
  MATCH_finished_jd: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_id; 
  
  MATCH_finished_id: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_hd; 
  
  MATCH_finished_hd: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 2 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16) + 
        addressToPC(MATCH_p);
      

       ret = pPcDisp(d16, delta, size);

      
      
      
    }
    
  }goto MATCH_finished_gd; 
  
  MATCH_finished_gd: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 3 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

      

              ret = pPcIndex(d8, iT, iR, iS, pc+2, size);

      
      
      
    }
    
  }goto MATCH_finished_fd; 
  
  MATCH_finished_fd: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_ed; 
  
  MATCH_finished_ed: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_dd; 
  
  MATCH_finished_dd: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 6 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 0 && 
      (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */ == 0 && 
      (1 <= (MATCH_w_16_0 >> 8 & 0x7) /* null at 0 */ && 
      (MATCH_w_16_0 >> 8 & 0x7) /* null at 0 */ < 8) || 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 0 && 
      (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */ == 1 || 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 1 || 
      1 <= (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ < 8) 
      goto MATCH_label_cd0;  /*opt-block+*/
    else { 
      unsigned d8 = (MATCH_w_16_0 & 0xff) /* disp8 at 0 */;
      

       ret = pImmB(d8);

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_cd; 
  
  MATCH_label_cd0: (void)0; /*placeholder for label*/ 
    

    pNonzeroByte(pc);

    
     
    goto MATCH_finished_cd; 
    
  MATCH_finished_cd: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 7 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned d16 = (MATCH_w_16_0 & 0xffff) /* d16 at 0 */;
      

       ret = pImmW(d16);

      
      
      
    }
    
  }goto MATCH_finished_bd; 
  
  MATCH_finished_bd: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 8 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pImmL(d32);

      
      
      
    }
    
  }goto MATCH_finished_ad; 
  
  MATCH_finished_ad: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::awlEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_zc0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: 
                

                 { mode = 5; result.numBytes += 4;}

                
                
                
                break;
              case 2: 
                

                 { mode = 2; result.numBytes += 2;}

                
                
                
                break;
              case 3: 
                

                 { mode = 3; result.numBytes += 2;}

                
                
                
                break;
              case 4: 
                if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
                  

                   { mode = 8; result.numBytes += 4;}

                  
                   /*opt-block+*/
                else 
                  

                   { mode = 7; result.numBytes += 2;}

                  
                   /*opt-block+*/
                
                break;
              case 5: case 6: case 7: 
                goto MATCH_label_zc0; break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_zc; 
  
  MATCH_label_zc0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_zc; 
    
  MATCH_finished_zc: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_yc; 
  
  MATCH_finished_yc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_xc; 
  
  MATCH_finished_xc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 2 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16) + 
        addressToPC(MATCH_p);
      

       ret = pPcDisp(d16, delta, size);

      
      
      
    }
    
  }goto MATCH_finished_wc; 
  
  MATCH_finished_wc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 3 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

      

              ret = pPcIndex(d8, iT, iR, iS, pc+2, size);

      
      
      
    }
    
  }goto MATCH_finished_vc; 
  
  MATCH_finished_vc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_uc; 
  
  MATCH_finished_uc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_tc; 
  
  MATCH_finished_tc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 7 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned d16 = (MATCH_w_16_0 & 0xffff) /* d16 at 0 */;
      

       ret = pImmW(d16);

      
      
      
    }
    
  }goto MATCH_finished_sc; 
  
  MATCH_finished_sc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 8 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pImmL(d32);

      
      
      
    }
    
  }goto MATCH_finished_rc; 
  
  MATCH_finished_rc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::cEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_qc0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: 
                

                 { mode = 5; result.numBytes += 4;}

                
                
                
                break;
              case 2: 
                

                 { mode = 2; result.numBytes += 2;}

                
                
                
                break;
              case 3: 
                

                 { mode = 3; result.numBytes += 2;}

                
                
                
                break;
              case 4: case 5: case 6: case 7: 
                goto MATCH_label_qc0; break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_qc; 
  
  MATCH_label_qc0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_qc; 
    
  MATCH_finished_qc: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_pc; 
  
  MATCH_finished_pc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_oc; 
  
  MATCH_finished_oc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 2 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned label = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16) + 
        addressToPC(MATCH_p);
      

       ret = pPcDisp(label, delta, size);

      
      
      
    }
    
  }goto MATCH_finished_nc; 
  
  MATCH_finished_nc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 3 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pPcIndex(d8, iT, iR, iS, pc+2, size);

      
      
      
    }
    
  }goto MATCH_finished_mc; 
  
  MATCH_finished_mc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_lc; 
  
  MATCH_finished_lc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_kc; 
  
  MATCH_finished_kc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  }  
  return ret;
}


SemStr* NJMCDecoder::dEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_jc0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: 
                

                 { mode = 5; result.numBytes += 4;}

                
                
                
                break;
              case 2: 
                

                 { mode = 2; result.numBytes += 2;}

                
                
                
                break;
              case 3: 
                

                 { mode = 3; result.numBytes += 2;}

                
                
                
                break;
              case 4: 
                
                  switch((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */) {
                    case 0: 
                      

                       { mode = 6; result.numBytes += 2;}

                      
                      
                      
                      break;
                    case 1: 
                      

                       { mode = 7; result.numBytes += 2;}

                      
                      
                      
                      break;
                    case 2: 
                      

                       { mode = 8; result.numBytes += 4;}

                      
                      
                      
                      break;
                    case 3: 
                      goto MATCH_label_jc0; break;
                    default: assert(0);
                  } /* (MATCH_w_16_0 >> 6 & 0x3) -- sz at 0 --*/ 
                break;
              case 5: case 6: case 7: 
                goto MATCH_label_jc0; break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_jc; 
  
  MATCH_label_jc0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_jc; 
    
  MATCH_finished_jc: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_ic; 
  
  MATCH_finished_ic: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_hc; 
  
  MATCH_finished_hc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 2 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned label = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16) + 
        addressToPC(MATCH_p);
      

       ret = pPcDisp(label, delta, size);

      
      
      
    }
    
  }goto MATCH_finished_gc; 
  
  MATCH_finished_gc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 3 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pPcIndex(d8, iT, iR, iS, pc+2, size);

      
      
      
    }
    
  }goto MATCH_finished_fc; 
  
  MATCH_finished_fc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_ec; 
  
  MATCH_finished_ec: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_dc; 
  
  MATCH_finished_dc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 6 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 0 && 
      (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */ == 0 && 
      (1 <= (MATCH_w_16_0 >> 8 & 0x7) /* null at 0 */ && 
      (MATCH_w_16_0 >> 8 & 0x7) /* null at 0 */ < 8) || 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 0 && 
      (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */ == 1 || 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 1 || 
      1 <= (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ < 8) 
      goto MATCH_label_cc0;  /*opt-block+*/
    else { 
      unsigned d8 = (MATCH_w_16_0 & 0xff) /* disp8 at 0 */;
      

       ret = pImmB(d8);

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_cc; 
  
  MATCH_label_cc0: (void)0; /*placeholder for label*/ 
    

    pNonzeroByte(pc);

    
     
    goto MATCH_finished_cc; 
    
  MATCH_finished_cc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 7 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned d16 = (MATCH_w_16_0 & 0xffff) /* d16 at 0 */;
      

       ret = pImmW(d16);

      
      
      
    }
    
  }goto MATCH_finished_bc; 
  
  MATCH_finished_bc: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 8 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pImmL(d32);

      
      
      
    }
    
  }goto MATCH_finished_ac; 
  
  MATCH_finished_ac: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  }  
  return ret;
}


SemStr* NJMCDecoder::daEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_zb0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: 
                

                 { mode = 5; result.numBytes += 4;}

                
                
                
                break;
              case 2: case 3: case 4: case 5: case 6: case 7: 
                goto MATCH_label_zb0; break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_zb; 
  
  MATCH_label_zb0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_zb; 
    
  MATCH_finished_zb: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_yb; 
  
  MATCH_finished_yb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_xb; 
  
  MATCH_finished_xb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_wb; 
  
  MATCH_finished_wb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_vb; 
  
  MATCH_finished_vb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::dBEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_ub0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: 
                

                 { mode = 5; result.numBytes += 4;}

                
                
                
                break;
              case 2: 
                

                 { mode = 2; result.numBytes += 2;}

                
                
                
                break;
              case 3: 
                

                 { mode = 3; result.numBytes += 2;}

                
                
                
                break;
              case 4: 
                

                 { mode = 6; result.numBytes += 2;}

                
                
                
                break;
              case 5: case 6: case 7: 
                goto MATCH_label_ub0; break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_ub; 
  
  MATCH_label_ub0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_ub; 
    
  MATCH_finished_ub: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_tb; 
  
  MATCH_finished_tb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_sb; 
  
  MATCH_finished_sb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 2 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned label = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16) + 
        addressToPC(MATCH_p);
      

       ret = pPcDisp(label, delta, size);

      
      
      
    }
    
  }goto MATCH_finished_rb; 
  
  MATCH_finished_rb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 3 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

      

              ret = pPcIndex(d8, iT, iR, iS, pc+2, size);

      
      
      
    }
    
  }goto MATCH_finished_qb; 
  
  MATCH_finished_qb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_pb; 
  
  MATCH_finished_pb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_ob; 
  
  MATCH_finished_ob: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 6 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 0 && 
      (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */ == 0 && 
      (1 <= (MATCH_w_16_0 >> 8 & 0x7) /* null at 0 */ && 
      (MATCH_w_16_0 >> 8 & 0x7) /* null at 0 */ < 8) || 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 0 && 
      (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */ == 1 || 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 1 || 
      1 <= (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ < 8) 
      goto MATCH_label_nb0;  /*opt-block+*/
    else { 
      unsigned d8 = (MATCH_w_16_0 & 0xff) /* disp8 at 0 */;
      

       ret = pImmB(d8);

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_nb; 
  
  MATCH_label_nb0: (void)0; /*placeholder for label*/ 
    

    pNonzeroByte(pc);

    
     
    goto MATCH_finished_nb; 
    
  MATCH_finished_nb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::dWEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_mb0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: 
                

                 { mode = 5; result.numBytes += 4;}

                
                
                
                break;
              case 2: 
                

                 { mode = 2; result.numBytes += 2;}

                
                
                
                break;
              case 3: 
                

                 { mode = 3; result.numBytes += 2;}

                
                
                
                break;
              case 4: 
                

                 { mode = 7; result.numBytes += 2;}

                
                
                
                break;
              case 5: case 6: case 7: 
                goto MATCH_label_mb0; break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_mb; 
  
  MATCH_label_mb0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_mb; 
    
  MATCH_finished_mb: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_lb; 
  
  MATCH_finished_lb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_kb; 
  
  MATCH_finished_kb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 2 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned label = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16) + 
        addressToPC(MATCH_p);
      

       ret = pPcDisp(label, delta, size);

      
      
      
    }
    
  }goto MATCH_finished_jb; 
  
  MATCH_finished_jb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 3 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

      

              ret = pPcIndex(d8, iT, iR, iS, pc+2, size);

      
      
      
    }
    
  }goto MATCH_finished_ib; 
  
  MATCH_finished_ib: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_hb; 
  
  MATCH_finished_hb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_gb; 
  
  MATCH_finished_gb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 7 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned d16 = (MATCH_w_16_0 & 0xffff) /* d16 at 0 */;
      

       ret = pImmW(d16);

      
      
      
    }
    
  }goto MATCH_finished_fb; 
  
  MATCH_finished_fb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::maEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_eb0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: 
                

                 { mode = 5; result.numBytes += 4;}

                
                
                
                break;
              case 2: case 3: case 4: case 5: case 6: case 7: 
                goto MATCH_label_eb0; break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_eb; 
  
  MATCH_label_eb0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_eb; 
    
  MATCH_finished_eb: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_db; 
  
  MATCH_finished_db: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_cb; 
  
  MATCH_finished_cb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_bb; 
  
  MATCH_finished_bb: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_ab; 
  
  MATCH_finished_ab: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::msEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_z0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: case 5: case 6: case 7: 
                goto MATCH_label_z0; break;
              case 2: 
                

                 { mode = 2; result.numBytes += 2;}

                
                
                
                break;
              case 3: 
                

                 { mode = 3; result.numBytes += 2;}

                
                
                
                break;
              case 4: 
                
                  switch((MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */) {
                    case 0: case 2: case 4: case 5: case 6: case 7: case 8: 
                    case 9: case 10: case 11: case 12: case 13: case 14: 
                    case 15: 
                      goto MATCH_label_z0; break;
                    case 1: 
                      

                       { mode = 6; result.numBytes += 2;}

                      
                      
                      
                      break;
                    case 3: 
                      

                       { mode = 7; result.numBytes += 2;}

                      
                      
                      
                      break;
                    default: assert(0);
                  } /* (MATCH_w_16_0 >> 12 & 0xf) -- op at 0 --*/ 
                break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_z; 
  
  MATCH_label_z0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_z; 
    
  MATCH_finished_z: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_y; 
  
  MATCH_finished_y: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_x; 
  
  MATCH_finished_x: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 2 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned label = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16) + 
        addressToPC(MATCH_p);
      

       ret = pPcDisp(label, delta, size);

      
      
      
    }
    
  }goto MATCH_finished_w; 
  
  MATCH_finished_w: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 3 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

      

              ret = pPcIndex(d8, iT, iR, iS, pc+2, size);

      
      
      
    }
    
  }goto MATCH_finished_v; 
  
  MATCH_finished_v: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_u; 
  
  MATCH_finished_u: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 6 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    if ((MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 0 && 
      (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */ == 0 && 
      (1 <= (MATCH_w_16_0 >> 8 & 0x7) /* null at 0 */ && 
      (MATCH_w_16_0 >> 8 & 0x7) /* null at 0 */ < 8) || 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 0 && 
      (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */ == 1 || 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ == 0 && 
      (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */ == 1 || 
      1 <= (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ && 
      (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */ < 8) 
      goto MATCH_label_t0;  /*opt-block+*/
    else { 
      unsigned d8 = (MATCH_w_16_0 & 0xff) /* disp8 at 0 */;
      

       ret = pImmB(d8);

      
      
      
    } /*opt-block*//*opt-block+*/
    
  }goto MATCH_finished_t; 
  
  MATCH_label_t0: (void)0; /*placeholder for label*/ 
    

    pNonzeroByte(pc);

    
     
    goto MATCH_finished_t; 
    
  MATCH_finished_t: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 7 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned d16 = (MATCH_w_16_0 & 0xffff) /* d16 at 0 */;
      

       ret = pImmW(d16);

      
      
      
    }
    
  }goto MATCH_finished_s; 
  
  MATCH_finished_s: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::msEAXL(ADDRESS eaxl, int d32, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eaxl
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
        case 0: case 2: case 3: case 5: case 6: case 7: 
          goto MATCH_label_r0; break;
        case 1: 
          if ((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 7) 
            

             { mode = 5; result.numBytes += 4;}

            
             /*opt-block+*/
          else 
            goto MATCH_label_r0;  /*opt-block+*/
          
          break;
        case 4: 
          if ((MATCH_w_16_0 >> 12 & 0xf) /* op at 0 */ == 2 && 
            (MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */ == 7) 
            

             { mode = 8; result.numBytes += 4;}

            
             /*opt-block+*/
          else 
            goto MATCH_label_r0;  /*opt-block+*/
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
    
  }goto MATCH_finished_r; 
  
  MATCH_label_r0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_r; 
    
  MATCH_finished_r: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 5 : {
      ret = pAbsL(d32, size);
      break;
    }
    case 8 : {
      ret = pImmL(d32);
      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::mdEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg1, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 6 & 0x7) /* MDadrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: case 7: 
          if ((MATCH_w_16_0 >> 8 & 0x1) /* sb at 0 */ == 1) 
            if ((MATCH_w_16_0 >> 6 & 0x3) /* sz at 0 */ == 3) 
              
                switch((MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */) {
                  case 0: 
                    

                     { mode = 4; result.numBytes += 2;}

                    
                    
                    
                    break;
                  case 1: 
                    

                     { mode = 5; result.numBytes += 4;}

                    
                    
                    
                    break;
                  case 2: case 3: case 4: case 5: case 6: case 7: 
                    goto MATCH_label_q0; break;
                  default: assert(0);
                } /* (MATCH_w_16_0 >> 9 & 0x7) -- reg1 at 0 --*/  
            else 
              goto MATCH_label_q0;  /*opt-block+*/ 
          else 
            goto MATCH_label_q0;  /*opt-block+*/
          break;
        case 5: 
          { 
            unsigned r1 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
            

             { mode = 0; reg1 = r1; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r1 = (MATCH_w_16_0 >> 9 & 0x7) /* reg1 at 0 */;
            

             { mode = 1; reg1 = r1; result.numBytes += 2;}

            
            
            
          }
          
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 6 & 0x7) -- MDadrm at 0 --*/ 
    
  }goto MATCH_finished_q; 
  
  MATCH_label_q0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_q; 
    
  MATCH_finished_q: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg1, size);

      
      
      
    }
    
  }goto MATCH_finished_p; 
  
  MATCH_finished_p: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg1, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_o; 
  
  MATCH_finished_o: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned d16 = (MATCH_w_16_0 & 0xffff) /* d16 at 0 */;
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_n; 
  
  MATCH_finished_n: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_m; 
  
  MATCH_finished_m: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::mrEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int delta, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_l0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: 
                

                 { mode = 5; result.numBytes += 4;}

                
                
                
                break;
              case 2: 
                

                 { mode = 2; result.numBytes += 2;}

                
                
                
                break;
              case 3: 
                

                 { mode = 3; result.numBytes += 2;}

                
                
                
                break;
              case 4: case 5: case 6: case 7: 
                goto MATCH_label_l0; break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_l; 
  
  MATCH_label_l0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_l; 
    
  MATCH_finished_l: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_k; 
  
  MATCH_finished_k: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_j; 
  
  MATCH_finished_j: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 2 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      unsigned label = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16) + 
        addressToPC(MATCH_p);
      

       ret = pPcDisp(label, delta, size);

      
      
      
    }
    
  }goto MATCH_finished_i; 
  
  MATCH_finished_i: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 3 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

      

              ret = pPcIndex(d8, iT, iR, iS, pc+2, size);

      
      
      
    }
    
  }goto MATCH_finished_h; 
  
  MATCH_finished_h: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_g; 
  
  MATCH_finished_g: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_f; 
  
  MATCH_finished_f: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}


SemStr* NJMCDecoder::rmEAX(ADDRESS eax, ADDRESS x, DecodeResult& result,
    ADDRESS pc, int size)
{
  SemStr* ret;
  int reg2, mode;




{ 
  dword MATCH_p = 
    

    eax
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    
      switch((MATCH_w_16_0 >> 3 & 0x7) /* adrm at 0 */) {
        case 0: case 1: case 2: case 3: case 4: 
          goto MATCH_label_e0; break;
        case 5: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 0; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 6: 
          { 
            unsigned r2 = (MATCH_w_16_0 & 0x7) /* reg2 at 0 */;
            

             { mode = 1; reg2 = r2; result.numBytes += 2;}

            
            
            
          }
          
          break;
        case 7: 
          
            switch((MATCH_w_16_0 & 0x7) /* reg2 at 0 */) {
              case 0: 
                

                 { mode = 4; result.numBytes += 2;}

                
                
                
                break;
              case 1: 
                

                 { mode = 5; result.numBytes += 4;}

                
                
                
                break;
              case 2: case 3: case 4: case 5: case 6: case 7: 
                goto MATCH_label_e0; break;
              default: assert(0);
            } /* (MATCH_w_16_0 & 0x7) -- reg2 at 0 --*/ 
          break;
        default: assert(0);
      } /* (MATCH_w_16_0 >> 3 & 0x7) -- adrm at 0 --*/ 
    
  }goto MATCH_finished_e; 
  
  MATCH_label_e0: (void)0; /*placeholder for label*/ 
    

    pIllegalMode(pc);

    
     
    goto MATCH_finished_e; 
    
  MATCH_finished_e: (void)0; /*placeholder for label*/
  
}



  switch (mode) {
    case 0 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pADisp(d16, reg2, size);

      
      
      
    }
    
  }goto MATCH_finished_d; 
  
  MATCH_finished_d: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 1 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~128..127] */ d8 = 
        sign_extend((MATCH_w_16_0 & 0xff) /* disp8 at 0 */, 8);
      unsigned iR = (MATCH_w_16_0 >> 12 & 0x7) /* iReg at 0 */;
      unsigned iS = (MATCH_w_16_0 >> 11 & 0x1) /* iSize at 0 */;
      unsigned iT = (MATCH_w_16_0 >> 15 & 0x1) /* iType at 0 */;
      

       ret = pAIndex(d8, reg2, iT, iR, iS, size);

      
      
      
    }
    
  }goto MATCH_finished_c; 
  
  MATCH_finished_c: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 4 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    { 
      int /* [~32768..32767] */ d16 = 
        sign_extend((MATCH_w_16_0 & 0xffff) /* d16 at 0 */, 16);
      

       ret = pAbsW(d16, size);

      
      
      
    }
    
  }goto MATCH_finished_b; 
  
  MATCH_finished_b: (void)0; /*placeholder for label*/
  
}


      break;
    }
    case 5 : {



{ 
  dword MATCH_p = 
    

    x
    ;
  unsigned /* [0..65535] */ MATCH_w_16_0;
  unsigned /* [0..65535] */ MATCH_w_16_16;
  { 
    MATCH_w_16_0 = getWord(MATCH_p); 
    MATCH_w_16_16 = getWord(2 + MATCH_p); 
    { 
      unsigned d32 = 
        ((MATCH_w_16_0 & 0xffff) /* d16 at 0 */ << 16) + 
        (MATCH_w_16_16 & 0xffff) /* d16 at 16 */;
      

       ret = pAbsL(d32, size);

      
      
      
    }
    
  }goto MATCH_finished_a; 
  
  MATCH_finished_a: (void)0; /*placeholder for label*/
  
}


      break;
    }
    default : pIllegalMode(pc); break;
  } 
  return ret;
}

/*==============================================================================
 * FUNCTION:      isFuncPrologue()
 * OVERVIEW:      Check to see if the instructions at the given offset match
 *                  any callee prologue, i.e. does it look like this offset
 *                  is a pointer to a function?
 * PARAMETERS:    hostPC - pointer to the code in question (native address)
 * \returns        True if a match found
 *============================================================================*/
bool isFuncPrologue(ADDRESS hostPC)
{
    int locals, reg, d16;

    if ((InstructionPatterns::link_save(prog.csrSrc, hostPC, locals, d16))
        != NULL)
            return true;
    if ((InstructionPatterns::link_save1(prog.csrSrc, hostPC, locals, reg))
        != NULL)
            return true;
    if ((InstructionPatterns::push_lea(prog.csrSrc, hostPC, locals, reg))
        != NULL)
            return true;
    if ((InstructionPatterns::std_link(prog.csrSrc, hostPC, locals)) != NULL)
        return true;

    return false;
}



