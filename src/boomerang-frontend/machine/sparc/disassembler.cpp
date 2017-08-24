#define sign_extend(N, SIZE)    (((int)((N) << (sizeof(unsigned) * 8 - (SIZE)))) >> (sizeof(unsigned) * 8 - (SIZE)))
#include <assert.h>

// #line 2 "machine/sparc/disassembler.m"

/***************************************************************************/ /**
 * \file       disassembler.m
 * OVERVIEW:   Skeleton file for a disassembler of SPARC instructions.
 *============================================================================*/

/*
 * $Revision$
 *
 *    Apr 01 - Cristina: Created
 * 18 May 01 - Mike: Slight mods to accomodate other disassemblers; moved
 *              default NJMCDecoder constructor to disasm.cc
 */

#include "boomerang/global.h"
#include "boomerang/include/decoder.h"
#include "boomerang/sparc-names.h"

// Globals in driver disasm.cc file
extern char _assembly[81];

// Need to provide the fetch routines for this machine (32-bits only
// on SPARC)

/*
 * FUNCTION:        getDword
 * OVERVIEW:        Returns the double starting at the given address.
 * PARAMETERS:      lc - address at which to decode the double
 * \returns          the decoded double
 */
DWord getDword(ADDRESS lc)
{
    Byte *p = (Byte *)lc;

    return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
}


/*
 * FUNCTION:         dis_RegImm
 * OVERVIEW:        decodes a register or an immediate value
 * PARAMETERS:         address pointer to be decoded
 * \returns          string with information about register or immediate
 */
char *NJMCDecoder::dis_RegImm(ADDRESS pc)
{
    static char _buffer[11];

    // #line 54 "machine/sparc/disassembler.m"
    {
        dword MATCH_p =

#line 54 "machine/sparc/disassembler.m"
            pc;
        unsigned MATCH_w_32_0;
        {
            MATCH_w_32_0 = getDword(MATCH_p);

            if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                int /* [~4096..4095] */ i = sign_extend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);

#line 55 "machine/sparc/disassembler.m"
                sprintf(_buffer, "%d", i);
            } /*opt-block*/ /*opt-block+*/
            else {
                unsigned rs2 = (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */;

#line 56 "machine/sparc/disassembler.m"
                sprintf(_buffer, "%s", DIS_RS2);
            } /*opt-block*/ /*opt-block+*/
        }
        goto MATCH_finished_c;

MATCH_finished_c:
        (void)0; /*placeholder for label*/
    }

    // #line 59 "machine/sparc/disassembler.m"
    return _buffer;
}


/*
 * FUNCTION:         dis_Eaddr
 * OVERVIEW:        decodes an effective address
 * PARAMETERS:         address pointer to be decoded
 * \returns          string with effective address in assembly format
 */
char *NJMCDecoder::dis_Eaddr(ADDRESS pc)
{
    static char _buffer[21];

    // #line 73 "machine/sparc/disassembler.m"
    {
        dword MATCH_p =

#line 73 "machine/sparc/disassembler.m"
            pc;
        unsigned MATCH_w_32_0;
        {
            MATCH_w_32_0 = getDword(MATCH_p);

            if ((MATCH_w_32_0 >> 13 & 0x1) /* i at 0 */ == 1) {
                if ((MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */ == 0) {
                    int /* [~4096..4095] */ i = sign_extend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);

#line 78 "machine/sparc/disassembler.m"
                    sprintf(_buffer, "[0x%x]", i);

                    strcat(constrName, "absoluteA ");
                } /*opt-block*/ /*opt-block+*/
                else {
                    int /* [~4096..4095] */ i   = sign_extend((MATCH_w_32_0 & 0x1fff) /* simm13 at 0 */, 13);
                    unsigned                rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;

#line 80 "machine/sparc/disassembler.m"
                    sprintf(_buffer, "[%s+%d]", DIS_RS1, i);

                    strcat(constrName, "dispA ");
                } /*opt-block*/ /*opt-block+*/ /*opt-block+*/
            }
            else if ((MATCH_w_32_0 & 0x1f) /* rs2 at 0 */ == 0) {
                unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;

#line 74 "machine/sparc/disassembler.m"
                sprintf(_buffer, "[%s]", DIS_RS1);

                strcat(constrName, "indirectA ");
            } /*opt-block*/ /*opt-block+*/
            else {
                unsigned rs1 = (MATCH_w_32_0 >> 14 & 0x1f) /* rs1 at 0 */;
                unsigned rs2 = (MATCH_w_32_0 & 0x1f) /* rs2 at 0 */;

#line 76 "machine/sparc/disassembler.m"
                sprintf(_buffer, "%s[%s]", DIS_RS1, DIS_RS2);

                strcat(constrName, "indexA ");
            } /*opt-block*/ /*opt-block+*/ /*opt-block+*/
        }
        goto MATCH_finished_b;

MATCH_finished_b:
        (void)0; /*placeholder for label*/
    }

    // #line 84 "machine/sparc/disassembler.m"
    return _buffer;
}
