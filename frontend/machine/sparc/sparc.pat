#
# Copyright (C) 1998-2000, The University of Queensland
# Copyright (C) 2001, Sun Microsystems, Inc
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

#==============================================================================
# FILE:       sparc.pat
# OVERVIEW:   A pattern specification for logues on the SPARC architecture.
#==============================================================================

# 07 Jan 00 - Mike: Added pattern for sethi/or/save and sethi/or/add
# 31 May 00 - Mike: Added pattern for the call/add caller prologue
# 25 Aug 00 - Mike: Pattern ret_imm_val can have the struct return form now
# 16 Mar 01 - Nathan: decode_sethi -> sethi to match a change in sparc-core.spec

NAMES

    SP = 14
    FP = 30
    o0 = 8
    i0 = 24
    i7 = 31
    o7 = 15
    g0 = 0

PATTERNS

###################
# Caller prologues.
###################

    # A call to a function that returns an integral type or nothing.
    CALLER_PROLOGUE std_call addr IS
        call__ (addr)

    # A call to a function that returns an aggregate type or quad
    # floating point value where imm22 is the size of the returned type.
    # The delay instruction must be re-decoded and have its corresponding
    # RTL built in the correct place.
    CALLER_PROLOGUE struct_call addr, imm22 IS
        call__(addr);
        <4>;
        UNIMP(imm22)

    # Exception! A call/restore never uses an UNIMP to specify that it
    # returns a struct (it returns whatever the caller returns, possibly a
    # struct, but if so, the caller's UNIMP would be used).
    # But we do see call/restores that happen to be followed by UNIMP inst-
    # ructions (e.g. case tables, where the table values are offsets). These
    # have to be treated as ordinary calls (for now). Example: setNAEOL() in
    # /usr/bin/vi (2.6).
    CALLER_PROLOGUE call_rst_ui_reg addr, imm22, rs1, rs2, rd IS
        call__ (addr);
        RESTORE (rs1, rmode(rs2), rd);
        UNIMP(imm22)

    # As above, but rs2 is replaced by imm
    CALLER_PROLOGUE call_rst_ui_imm addr, imm22, rs1, imm, rd IS
        call__ (addr);
        RESTORE (rs1, imode(imm), rd);
        UNIMP(imm22)

    # Call/restore. The registers have to be handled delicately, because the
    # restore has an implicit "add" in it, which reads from the old register
    # window, and writes to the new. Also, parameters are (usually) passed in
    # I registers (except where the restore "carries" them to O regs)
    CALLER_PROLOGUE call_restore_reg addr, rs1, rs2, rd IS
        call__ (addr);
        RESTORE (rs1, rmode(rs2), rd)

    # As above, but with rs2 replaced by imm
    CALLER_PROLOGUE call_restore_imm addr, rs1, imm, rd IS
        call__ (addr);
        RESTORE (rs1, imode(imm), rd)

    # In a leaf procedure, you can occasionally find this type of idiom:
    # mov %o7,%g1
    # call called
    # mov %g1,%o7
    # Of course, various registers (usually g registers) can be used to save
    # the return address. The effect is to jump to called, so it's the leaf
    # optimised equivalent of call/restore
    # This caller prologue is also a callee epilogue!
    CALLER_PROLOGUE move_call_move addr, rd IS
        mov_ ($o7, rd);
        call__ (addr);
        mov_ (rd, $o7)

    # The above can get mangled into a move/x/call/move where x is just some
    # instruction that was moved for optimisation.
    # The semantics is that of x, followed by the move_call_move semantics
    # It is assumed that the instruction won't affect %o7 or rd.
    # The second instruction has to be re-decoded on its own
    CALLER_PROLOGUE move_x_call_move addr, rd IS
        mov_ ($o7, rd);
        <4>;
        call__ (addr);
        mov_ (rd, $o7)

    # Only seen from gcc compiler: call/add where rs1 and dest are %o7. This
    # makes the call/add pair effectively a call/branch sequence, skipping
    # simm13 bytes of code after the call
    CALLER_PROLOGUE call_add addr, imm IS
        call__ (addr);
        ADD ($o7, imode(imm), $o7)

    # Another idiomatic pattern: jmp/restore. This is just like call/restore,
    # but it's used for computed calls
    # This caller prologue is also a callee epilogue!
    CALLER_PROLOGUE jmp_restore_reg rs1j, rdj, rs1, rs2, rd IS
        JMPL (indirectA(rs1j), rdj);
        RESTORE (rs1, rmode(rs2), rd)
    # As above, but with immediate
    CALLER_PROLOGUE jmp_restore_imm rs1j, rdj, rs1, imm, rd IS
        JMPL (indirectA(rs1j), rdj);
        RESTORE (rs1, imode(imm), rd)

