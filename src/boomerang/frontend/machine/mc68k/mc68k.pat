#
# Copyright (C) 2000, The University of Queensland
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

#==============================================================================
# FILE:       mc68k.pat
# OVERVIEW:   A pattern specification for logues on the MC68000 architecture.
#==============================================================================

# $Revision$
# 09 Feb 2000 - Mike: Started
# 16 Feb 2000 - Mike: Renamed CALLEE_PROLOGUE link to std_link (was causing
#                     toolkit to get confused)

NAMES

    SP = 7              # Note: these are A register numbers, not SSL nums!
    FP = 6

PATTERNS

###################
# Caller prologues.
###################

    # Jump to subroutine, pc relative (2 word instruction)
    CALLER_PROLOGUE std_call addr IS
        call_ (addr)

    # Branch to subroutine, pc relative (one word instruction)
    CALLER_PROLOGUE near_call addr IS
        bsr    (addr)

    # Pea/add to tos/rts pattern
    # This pattern uses a new constructor just for the pea/PcDisp combination
    CALLER_PROLOGUE pea_add_rts d32 IS
        peaPcDisp(4);
        addil   (d32, daIndirect($SP));
        rts()

    # Pea/pea/add/rts pattern: this appears to be a "long call"
    CALLER_PROLOGUE pea_pea_add_rts d32 IS
        peaPcDisp(0xE);
        peaPcDisp(4);
        addil   (d32, daIndirect($SP));
        rts()

    # Trap / system calls
    CALLER_PROLOGUE trap_syscall d16 IS
        trap(15);
        Aline(d16)

###################
# Callee prologues.
###################

    # Standard procedure, with stack allocation
    CALLEE_PROLOGUE std_link locals IS
        link    ($FP, locals)

    # Standard procedure, with link and movem to save registers
    CALLEE_PROLOGUE link_save locals, d16 IS
        link    ($FP, locals);
        moverml (d16, daPreDec($SP))

    # Standard procedure, but with only one register saved (so uses a move
    # rather than a movem instruction)
    CALLEE_PROLOGUE link_save1 locals, reg IS
        link    ($FP, locals);
        pushreg (reg)

    # An optimised function might have just a push and an lea as a prologue
    CALLEE_PROLOGUE push_lea locals, reg IS
        pushreg (reg);
        leaSpSp (locals)

    # A function can be just a bare return statement!
    CALLEE_PROLOGUE bare_ret IS
        rts()

###################
# Callee epilogues.
###################

    # Callee returns and restores the register window. The return
    # value or the pointer to the returned struct (if any) is
    # already be in the correct register.
    CALLEE_EPILOGUE std_ret IS
        unlk($FP);
        rts()

    # Restore resisters and standard return
    CALLEE_EPILOGUE rest_ret d16 IS
        movemrl (daPostInc($SP), d16);
        unlk($FP);
        rts()

    # Restore 1 resister and standard return
    CALLEE_EPILOGUE rest1_ret reg IS
        popreg  (reg);
        unlk($FP);
        rts()

    # Restore 1 register, NO unlink, and return
    CALLEE_EPILOGUE pop_ret reg IS
        popreg  (reg);
        rts()

###################
# Caller epilogues.
###################

    # Standard epilogue (may not be present) is to remove n bytes from the
    # stack. This is usually done with "addql #nn, %a7", (or addqw, same
    # semantics) or with an "lea nn(a7), a7" instruction.
    # Gcc likes to use addaw
    # Note: we don't use a pattern for addq any more, since that way we bypass
    # the decoder, and hence the "0 really means 8" semantics.
    # The analysis code doesn't really need these epilogues anyway
    CALLER_EPILOGUE clear_stack n IS
        [
#         addql     (n, alADirect($SP)) |
#         addqw     (n, alADirect($SP)) |
          leaSpSp   (n) |
          addaw_d16 (n) ]

