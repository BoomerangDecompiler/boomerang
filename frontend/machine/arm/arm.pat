#
# Copyright (C) 2001, Sun Microsystems, Inc
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

#==============================================================================
# FILE:       arm.pat
# OVERVIEW:   A pattern specification for logues for the ARM architecture
#             with no hardware floating point support. An example of such
#             a processor is the StrongARM. This specification does not
#             include support for the THUMB architecture. It also assumes
#             the prologues and epilogues emitted by gcc.
#============================================================================*/
# 08 Aug 01 - Brian: Created file.
# 20 Aug 01 - Cristina: fixed prologues that return "num_saved_regs" by 
#             adding a new keyword to PAL to count the number of such registers.

NAMES

    PC = 15
    LR = 14
    SP = 13
    IP = 12
    FP = 11

PATTERNS

###################
# Caller prologues.
###################

    # A call to a function using a bl (branch and link) instruction. This is
    # used for destinations within a 32MB range.
    CALLER_PROLOGUE std_proc_call addr IS
        bl (addr)

    # A "long call" to a function done by setting the PC to a destination
    # address. This must be proceeded by an instruction that sets the LR.
    # This supports destinations anywhere in the 4GB address space.
    CALLER_PROLOGUE long_proc_call addr IS
        mov ($LR, $PC);
        [ ldr ($PC, rb, imm12) |
          mov ($PC, rb, ?) ]

###################
# Callee prologues.
###################

    # Callee allocates space for the stack and saves registers. A standard set
    # of four registers is saved on the stack (FP, IP, LR, PC). Additional
    # non-scratch registers (r4-r11) can also be saved using the same stmfd
    # instruction. The ARM decoder.m returns the number of registers saved
    # by stmfd in the variable "num_saved_regs".
    CALLEE_PROLOGUE std_entry local_vars, num_saved_regs IS
        mov ($IP, $SP);
        stmfd_update ($SP, rn, reg-list) *num_saved_regs = POP-COUNT(reg-list);
        sub ($FP, $IP, 4);
        sub ($SP, $SP, local_vars)

    # If a double-word argument is "split" between r3 and the stack, gcc
    # reserves space on the stack to store the low-order word passed in r3
    # so that it can later read the entire argument using a doubleword load.
    CALLEE_PROLOGUE split_arg_entry local_vars num_saved_regs IS
        mov ($IP, $SP);
        stmfd_update ($SP, rn, reg-list) *num_saved_regs = POP-COUNT(reg-list);
        sub ($FP, $IP, 8);
        sub ($SP, $SP, local_vars)

###################
# Callee epilogues.
###################

    # Callee returns by restoring saved registers from the stack. This includes
    # resetting PC (causing the return), SP, and FP. Additional non-scratch
    # registers (r4-r11) can also be restored using the same ldmea instruction. 
    CALLEE_EPILOGUE std_ret IS
        ldmea ($FP, rn, reg-list) *num_saved_regs = POP-COUNT(reg-list);  

