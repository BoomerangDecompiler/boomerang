#
# Copyright (C) 2001, The University of Queensland
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

#==============================================================================
# FILE:       hppa.pat
# OVERVIEW:   A pattern specification for logues on the HP PA/RISC architecture.
#============================================================================*/

# $Revision$
#
# 27 Apr 01 - Mike: Created
# 02 May 01 - Mike: Added gcc_frame
# 09 May 01 - Mike: Various fiddly changes; matches the 3 logues in hello world
# 14 May 01 - Mike: gcc_frame prologue does not include register saves now (need
#               to see and translate these saves)
# 17 May 01 - Mike: Added gcc frameless logues
# 28 Jun 01 - Mike: Modified for 1.1 style opcodes
# 17 Jul 01 - Simon: Mods to match changes to hppa.spec
# 30 Jul 01 - Mike: Mod to gcc_unframeless to make a register restore optional
# 01 Aug 01 - Mike: Fixed missing dollar in std_call pattern
# 07 Aug 01 - Mike: Added patterns bare_ret and bare_ret_anulled
# 07 Aug 01 - Simon: Loads and stores take more parameters now
# 20 Aug 01 - Mike: Added param_reloc1 pattern for common printf of floats

NAMES

    R0 = 0
    RP = 2
    SP = 30
    R31 = 31

PATTERNS

###################
# Caller prologues.
###################

    # A call to a function that returns an integral type or nothing.
    CALLER_PROLOGUE std_call addr IS
        BL (c_br_nnull(), addr, $RP)

###################
# Caller prologues.
###################


###################
# Callee prologues.
###################
    # Gcc style, with a frame pointer
    CALLEE_PROLOGUE gcc_frame locals IS
        STW (c_l_addr_none(), $RP, l_addr_16_old(-20), 0, $SP);
        OR (c_arith_w(0, c_c_nonneg()), 3, 0, 1);
        OR (c_arith_w(0, c_c_nonneg()), $SP, 0, 3);
        STWM(c_l_addr_none(), 1, l_addr_16_old(locals), 0, $SP);
      { STW (c_l_addr_none(), 4, l_addr_16_old(8), 0, 3) } 

    # Gcc style, optimised (no frame)
    CALLEE_PROLOGUE gcc_frameless locals IS
        STW (c_l_addr_none(), $RP, l_addr_16_old(-20), 0, $SP);
      [ LDO (locals, $SP, $SP) |
        STWM(c_l_addr_none(), ?, l_addr_16_old(locals), 0, $SP) ];
      { STW (c_l_addr_none(), ?, l_addr_16_old(?), 0, $SP) }
    
    # hmm, this *might* be a leaf procedure,
    # but I've only seen it for transitives for now
#    CALLEE_PROLOGUE gcc_transition IS
#    <0>

    # A parameter relocation stub, commonly seen when a floating point
    # argument is the second parameter to printf
    # This prologue is for the whole stub, so it's really a prologue and
    # epilogue combined
    # The library that is branched to at the end has address PC + libstub
    # There are no locals for this procedure, but all CALLEE_PROLOGUEs need
    # a locals parameter
    CALLEE_PROLOGUE param_reloc1 libstub, locals IS
        FSTDS (c_s_addr_ma(),   7, s_addr_im_r(8), 0, $SP);
        LDWS  (c_s_addr_notm(), s_addr_im_r(-4), 0, $SP, 24);
        LDWS  (c_s_addr_mb(),   s_addr_im_r(-8), 0, $SP, 23);
        BL    (c_br_null(), libstub, 0)
    
###################
# Callee epilogues.
###################
    # Gcc style, with frame pointer
    CALLEE_EPILOGUE gcc_unframe IS
      { LDW (c_l_addr_none(), l_addr_16_old(-20), 0, 3, $RP) };
      { LDW (c_l_addr_none(), l_addr_16_old(8), 0, 3, 4) };
        LDO  (?, 3, $SP);
        LDWM(c_l_addr_none(), l_addr_16_old(?), 0, $SP, 3);
        BV (c_br_null(), $R0, $RP)

    # Gcc style, optimised (no frame pointer)
    CALLEE_EPILOGUE gcc_unframeless1 IS
       { LDW (c_l_addr_none(), l_addr_16_old(?), 0, $SP, $RP) };
       { LDW (c_l_addr_none(), l_addr_16_old(?), 0, $SP, ? ) };
        BV (c_br_nnull(), $R0, $RP);
        LDWM(c_l_addr_none(), l_addr_16_old(?), 0, $SP, ?)

    CALLEE_EPILOGUE gcc_unframeless2 IS
      { LDW (c_l_addr_none(), l_addr_16_old(?), 0, $SP, 4) };
        LDW (c_l_addr_none(), l_addr_16_old(?), 0, $SP, 3);
        BV (c_br_nnull(), $R0, $RP);
        LDO  (?, $SP, $SP)

    # Bare return statement (not anulled) (match almost last)
    CALLEE_EPILOGUE bare_ret IS
        BV (c_br_nnull(), $R0, $RP)

    # Bare return statement (anulled) (match last)
    CALLEE_EPILOGUE bare_ret_anulled IS
        BV (c_br_null(), $R0, $RP)


