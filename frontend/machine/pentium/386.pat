#
# Copyright (C) 1998-2001, The University of Queensland
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

#==============================================================================
# FILE:       386.pat
# OVERVIEW:   A pattern specification for logues on the 386 architecture.
#============================================================================*/

# 14 Jul 00 - Mike: Added patterns for strlen, memcpy
# 08 Mar 01 - Mike: Added patterns for frameless_pro, frameless_epi
# 26 Mar 01 - Mike: Added callee prologue "none"
# 30 Mar 01 - Mike: Merged "frameless" and local-less" patterns into one
# 25 Apr 01 - Mike: REP and REPNE are part of the string instr now
# 26 Oct 01 - Mike: Commented out a line in the standard callee epilogue that
#               matches a move from the stack to %eax (e.g. see returnparam)
# 01 Feb 02 - Mike: Added pat_this_thunk off pattern

NAMES
    EAX = 0
    ECX = 1
    EDX = 2
    EBX = 3
    ESP = 4
    EBP = 5
    ESI = 6
    EDI = 7

PATTERNS

###################
# Caller prologues.
###################

    # A call to a function at a fixed address
    CALLER_PROLOGUE std_call addr IS
        CALL.Jvod (addr)

    # A pattern for an optimisation of strlen
    # Translates to eax := 0; ecx := -2 -(strlen(%edi)
    CALLER_PROLOGUE pat_strlen IS
        XORrmod($EAX, Reg($EAX));
        CLD();
        MOVid($ECX, -1);
        REPNE.SCASB()

    # A set of 4 patterns for memcpy. We need 4 so that moves of other than
    # multiples of 4 bytes are handled as one call, not 2 or even 3
    CALLER_PROLOGUE pat_memcpy_00 len IS
        CLD();
        MOVid($ECX, len);
        REP.MOVSvod()

    # As above, but 1 byte left over to move
    CALLER_PROLOGUE pat_memcpy_01 len IS
        CLD();
        MOVid($ECX, len);
        REP.MOVSvod();
        MOVSB()

    # As above, but 1 word left over to move
    CALLER_PROLOGUE pat_memcpy_10 len IS
        CLD();
        MOVid($ECX, len);
        REP.MOVSvod();
        MOVSvow()

    # As above, but 1 word and 1 byte left over to move
    CALLER_PROLOGUE pat_memcpy_11 len IS
        CLD();
        MOVid($ECX, len);
        REP.MOVSvod();
        MOVSvow();
        MOVSB()

    # A thunk, used in C++ programs to adjust the "this" pointer
    # This pattern assumes that the fixup fits into a byte (magnitude <= 128)
    CALLER_PROLOGUE pat_this_thunk off, dest IS
        ADDiodb (E( Base8(4, $ESP)), off);
        JMP.Jvod (dest)         # This is the only type of jump seen so far


###################
# Callee prologues.
###################

    # Standard callee prologue: standard call frame, preserves regs
    # registers.
    CALLEE_PROLOGUE std_entry locals=0, regs IS
        PUSHod ($EBP);
        { MOVrmod ($EBP, Reg($ESP)) |
          MOVmrod (Reg($EBP), $ESP) };
        { SUBiodb (Reg ($ESP), locals) |
		  SUBid	  (Reg ($ESP), locals) };
        { [ PUSHod ($ESI) |
            PUSHod ($EBX) |
            PUSHod ($EDI) ] *regs <1..3> }

    # Preserves regs registers. Removes pointer to return struct space
    # and puts it into a register.
    CALLEE_PROLOGUE struct_ptr locals, regs IS
        POPod ($EAX);
        XCHG.Ev.Gvod (E (Base ($ESP)), $EAX);
        @std_entry (locals, regs)

    # Prologue for frameless procedures (e.g. ninths.1 test)
    # Does not set up the frame pointer %ebp. 
    CALLEE_PROLOGUE frameless_pro locals, regs IS
        { SUBiodb (Reg($ESP), locals) | SUBid (Reg($ESP), locals) };
        { [ PUSHod ($ESI) |
            PUSHod ($EBX) |
            PUSHod ($EDI) ] *regs <1..3> }

    # Sometimes, there isn't a prologue at all
    # Using the CALLER spec isn't any good when passing through stack
    CALLEE_PROLOGUE none IS
    <0>

###################
# Callee epilogues.
###################

    # Standard epilogue that restores any
    # preserved registers. If the return instruction is of the RET.Iw
    # form, then this is a callee returning a struct.
    CALLEE_EPILOGUE std_ret IS
#        { MOVrmod ($EAX, E(Disp8(?,$EBP))) };
        { LEAod ($ESP, Disp8(?,$EBP)) };
        { [ MOVrmod ($EBX, E( Disp8(?,$EBP))) |
            MOVrmod ($ESI, E( Disp8(?,$EBP))) |
            MOVrmod ($EDI, E( Disp8(?,$EBP))) ] * <1..3> 
          |
          [ POPod ($EBX) |
            POPod ($ESI) |
            POPod ($EDI) ] *<1..3> };
        [ LEAVE () | [ MOVrmod ($ESP, Reg($EBP)); POPod ($EBP) ]];
        [ RET () | RET.Iw (?) ]

    # Epilogue for frameless procedures
    CALLEE_EPILOGUE frameless_epi n IS
        { [ POPod ($EBX) |
            POPod ($ESI) |
            POPod ($EDI) ] *<1..3> };
        { [ ADDiodb (Reg($ESP),n) | ADDid (Reg($ESP),n) ] };
        [ RET () | RET.Iw (?) ]

    # Catch all for bare returns
    # Note that the above will always match before this one now
    # CALLEE_EPILOGUE simple_ret IS
    #    [ RET () | RET.Iw (?) ]

###################
# Caller epilogues.
###################

    # Standard epilogue (may not be present) is to remove n bytes from the
    # stack. This can be done either with an addition to the stack
    # pointer or by a number of pops.
    CALLER_EPILOGUE clear_stack n IS
        [ ADDiodb (Reg($ESP),n) | ADDid (Reg($ESP),n) ] |
        [ POPod ($EAX) |
          POPod ($EBX) |
          POPod ($ECX) |
          POPod ($EDX) |
          POPod ($ESI) |
          POPod ($EDI) |
          POPod ($EBP) ] * n <1..7>
