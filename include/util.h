/*==============================================================================
 * FILE:       util.h
 * OVERVIEW:   Provides the definition for the miscellaneous bits and pieces
 *               implemented in the util.so library
 *============================================================================*/
/*
 * $Revision$
 *
 * 10 Apr 02 - Mike: Created
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <sstream>
#include <string>
#include "types.h"

// was a workaround
#define STR(x) (char *)(x.str().c_str())
// Upper case a C string: s is source, d is dest
void upperStr(const char* s, char* d);
// Add string and integer
std::string operator+(const std::string& s, int i);

// load / save functions
void load(std::string &location);
void save(std::string &location);

// helper functions for load/save
void loadString(std::istream &is, std::string &str);
void saveString(std::ostream &os, const std::string &str);
int loadFID(std::istream &is);
void saveFID(std::ostream &os, int fid);
void skipFID(std::istream &is, int fid);
int loadLen(std::istream &is);
void saveLen(std::ostream &os, int len, bool large = false);
void loadValue(std::istream &is, bool &b, bool wantlen = true);
void saveValue(std::ostream &os, bool b, bool wantlen = true);
void loadValue(std::istream &is, char &ch, bool wantlen = true);
void saveValue(std::ostream &os, char ch, bool wantlen = true);
void loadValue(std::istream &is, int &i, bool wantlen = true);
void saveValue(std::ostream &os, int i, bool wantlen = true);
void loadValue(std::istream &is, ADDRESS &a, bool wantlen = true);
void saveValue(std::ostream &os, ADDRESS a, bool wantlen = true);
void loadValue(std::istream &is, double &d, bool wantlen = true);
void saveValue(std::ostream &os, double d, bool wantlen = true);

// ids used in binary save file format
// DO NOT RENUMBER THESE DEFINITIONS
// if you want to reorder them in the header so they are better grouped
// ensure that each identifier is still unique.
#define FID_MAGIC "BPF"
#define FID_MAGIC_LEN strlen(FID_MAGIC)
#define FID_VERSION 1 // this version number should only be increased when compatibility is broken

#define FID_PROJECT_NAME	0      // name of the project
#define FID_FILENAME		1      // filename of source binary (that was decompiled)
#define FID_SYMBOL			25	   // global symbol
#define FID_FRONTEND		27	   // name of frontend used

#define FID_PROC			2      // procedure
#define FID_PROC_DECODED    3      // boolean to indicate if this procedure is decoded
#define FID_PROC_FIRSTCALLER 13	   // indicates the first proc to call this proc
#define FID_PROC_CALLEE		14	   // a callee of this proc
#define FID_PROC_SYMBOL		26	   // local symbol
#define FID_PROC_SIGNATURE  30	   // signature of this proc
#define FID_PROC_END		4	   // indicates the end of a procedure

#define FID_CFG				5      // a control flow graph
#define FID_CFG_WELLFORMED  6      // boolean to indicate if the cfg is well formed
#define FID_CFG_BB			7	   // basic block
#define FID_CFG_ENTRYBB     8      // indicates the entry bb of a cfg
#define FID_CFG_END			9	   // indicates the end of the CFG

#define FID_BB_TYPE			10     // indicates the type of a BB
#define FID_BB_OUTEDGES		11	   // specifies an array of numerical outedges of the bb
#define FID_BB_RTL			15	   // specifies an rtl
#define FID_BB_END			12	   // indicates the end of a BB

#define FID_RTL_NUMNATIVEBYTES 17  // number of native bytes in source instruction
#define FID_RTL_JCOND		28	   // jcond expression
#define FID_RTL_JCONDTYPE	18	   // indicates the type of jcond
#define FID_RTL_USESFLOATCC 19	   // indicates the jcond uses float condition codes
#define FID_RTL_EXP			22	   // indicates an exp in this rtl
#define FID_RTL_FIXDEST		23	   // fixed destination of a jump
#define FID_RTL_JDEST		24	   // unfixed destination of a jump
#define FID_RTL_CALLDESTSTR 29     // name of the destination proc of a HLCall
#define FID_RTL_END			16	   // indicates the end of an RTL

#define FID_EXP_END			20	   // indicates the end of an EXP

#define FID_TYPE_SIZE		33	   // indicates the size of a TYPE
#define FID_TYPE_SIGN		34	   // indicates the sign of a TYPE
#define FID_TYPE_SIGNATURE	35	   // indicates the signature of a func type
#define FID_TYPE_END		21	   // indicates the end of an TYPE

#define FID_SIGNATURE_PARAM 31	   // a parameter of a signature
#define FID_SIGNATURE_END	32	   // indicates the end of a SIGNATURE

// update this as you go.
#define FID_LAST			36

#endif
