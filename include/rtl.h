/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************//**
 * \file       rtl.h
 * Definition of the classes that describe an RTL, a low-level
 *       register transfer list. Higher-level RTLs (instance
 *       of class HLJump, HLCall, etc.) represent information about
 *       a control transfer instruction (CTI) in the source program.
 *       analysis code adds information to existing higher-level
 *       RTLs and sometimes creates new higher-level RTLs (e.g., for
 *       switch statements).
 *============================================================================*/

/*
 * $Revision$    // 1.51.2.1
 *
 * 08 Apr 02 - Mike: Mods for boomerang
 * 13 May 02 - Mike: expList is no longer a pointer
 * 25 Jul 03 - Mike: RTL now a list of Statements
 */

#ifndef __RTL_H__
#define __RTL_H__

#include <list>
#include <vector>
#include <map>
#include <set>
#include <iosfwd>
#include "type.h"
//#include "exp.h"
#include "register.h"

class BasicBlock;
class HLLCode;
class Exp;
class TypedExp;
class DefSet;
class UseSet;
class Type;
class Register;
class Proc;
class XMLProgParser;
class StmtVisitor;
class Statement;
enum STMT_KIND : uint8_t;

/***************************************************************************//**
 * Class RTL: describes low level register transfer lists (actually lists of statements).
 * NOTE: when time permits, this class could be removed, replaced with new Statements that mark the current native
 * address
 *============================================================================*/
class RTL : public std::list<Statement*> {
        ADDRESS     nativeAddr;                            // RTL's source program instruction address
public:
                    RTL();
                    RTL(ADDRESS instNativeAddr, std::list<Statement*>* listStmt = nullptr);
                    RTL(const RTL& other);                    // Makes deep copy of "other"

        RTL *       clone() const;
        RTL &       operator=(const RTL &other);

        // Common enquiry methods
        ADDRESS     getAddress() {return nativeAddr;}       //!< Return RTL's native address
        void        setAddress(ADDRESS a) {nativeAddr=a;}   //!< Set the address
        bool        areFlagsAffected();                     //!< True if flags are affected

        // Statement list editing methods
        void        appendStmt(Statement *s);                // Add s to end of RTL.
        void        appendListStmt(std::list<Statement*>& le);
        // Make a deep copy of the list of Exp*
        void        deepCopyList(std::list<Statement*>& dest);

         // Print RTL to a stream.
        void        print(std::ostream& os = std::cout, bool html = false) const;
        void        dump();

        bool        isCall(); // Is this RTL a call instruction?
        Statement * getHlStmt();
        char *      prints() const; // Print to a string (mainly for debugging)
protected:
        void        simplify();
        friend class XMLProgParser;
        friend class BasicBlock;
};



/***************************************************************************//**
 * The TableEntry class represents a single instruction - a string/RTL pair.
 *
 * This class plus ParamEntry and RTLInstDict should be moved to a separate
 * header file...
 *============================================================================*/
class TableEntry {
public:
    TableEntry();
    TableEntry(std::list<std::string>& p, RTL& rtl);

    const TableEntry& operator=(const TableEntry& other);

    void setParam(std::list<std::string>& p);
    void setRTL(RTL& rtl);

    // non-zero return indicates failure
    int appendRTL(std::list<std::string>& p, RTL& rtl);

public:
    std::list<std::string> params;
    RTL rtl;

#define TEF_NEXTPC 1
    int flags;                    // aka required capabilities. Init. to 0
};


/***************************************************************************//**
 * The ParamEntry class represents the details of a single parameter.
 *============================================================================*/
typedef enum {PARAM_SIMPLE, PARAM_ASGN, PARAM_LAMBDA, PARAM_VARIANT} ParamKind;

class ParamEntry {
public:
        ParamEntry() {
            asgn = nullptr;
            kind = PARAM_SIMPLE;
            type = nullptr;
            regType = nullptr;
            lhs = false;
            mark = 0;
        }
        ~ParamEntry() {
            delete type;
            delete regType;
        }

        std::list<std::string> params;        //!< PARAM_VARIANT & PARAM_ASGN only */
        std::list<std::string> funcParams;    //!< PARAM_LAMBDA - late bound params */
        Statement *     asgn;                    //!< PARAM_ASGN only */
        bool            lhs;                    //!< True if this param ever appears on the LHS of an expression */
        ParamKind       kind;
        Type *          type;
        Type *          regType;                //!< Type of r[this], if any (void otherwise)
        std::set<int>   regIdx;                //!< Values this param can take as an r[param]
        int             mark;                    //!< Traversal mark. (free temporary use, basically)
};


/***************************************************************************//**
 * The RTLInstDict represents a dictionary that maps instruction names to the
 * parameters they take and a template for the Exp list describing their
 * semantics. It handles both the parsing of the SSL file that fills in
 * the dictionary entries as well as instantiation of an Exp list for a given
 * instruction name and list of actual parameters.
 *============================================================================*/

class RTLInstDict {
public:
        RTLInstDict();
        ~RTLInstDict();

        bool    readSSLFile(const std::string& SSLFileName);
        void    reset();
        std::pair<std::string,unsigned> getSignature(const char* name);

        int appendToDict(std::string &n, std::list<std::string>& p, RTL& rtl);

        // Given an instruction name and list of actual parameters, return an instantiated RTL for the corresponding
        // instruction entry.
        std::list<Statement*>* instantiateRTL(std::string& name, ADDRESS natPC, std::vector<Exp*>& actuals);
        // As above, but takes an RTL & param list directly rather than doing a table lookup by name.
        std::list<Statement*>* instantiateRTL(RTL& rtls, ADDRESS natPC, std::list<std::string> &params,
            std::vector<Exp*>& actuals);

        void transformPostVars(std::list<Statement*> &rts, bool optimise);

        void        print(std::ostream& os = std::cout);

        // Add a new register to the machine
        void        addRegister(const char *name, int id, int size, bool flt);

        // If the top level operator of the given expression indicates any kind of type, update ty to match.
        bool        partialType(Exp* exp, Type& ty);

    // Go through the params and fixup any lambda functions
    void            fixupParams();

public:
    // A map from the symbolic representation of a register (e.g. "%g0") to its index within an array of registers.
    std::map<std::string, int, std::less<std::string> > RegMap;

    // Similar to r_map but stores more info about a register such as its size, its addresss etc (see register.h).
    std::map<int, Register, std::less<int> > DetRegMap;

    // A map from symbolic representation of a special (non-addressable) register to a Register object
    std::map<std::string, Register, std::less<std::string> > SpecialRegMap;

    // A set of parameter names, to make sure they are declared (?).
    // Was map from string to SemTable index
    std::set<std::string> ParamSet;

    // Parameter (instruction operand, more like addressing mode) details (where given)
    std::map<std::string, ParamEntry> DetParamMap;

    // The maps which summarise the semantics (.ssl) file
    std::map<std::string, Exp*> FlagFuncs;
    std::map<std::string, std::pair<int, void*>*> DefMap;
    std::map<int, Exp*> AliasMap;

    // Map from ordinary instruction to fast pseudo instruction, for use with -f (fast but not as exact) switch
    std::map<std::string, std::string> fastMap;

    bool bigEndian;                   // True if this source is big endian

    // The actual dictionary.
    std::map<std::string, TableEntry, std::less<std::string> > idict;

    // An RTL describing the machine's basic fetch-execute cycle
    std::list<Statement*>*fetchExecCycle;

    void fixupParamsSub(std::string s, std::list<std::string>& funcParams, bool& haveCount, int mark);
};

#endif /*__RTL_H__*/

