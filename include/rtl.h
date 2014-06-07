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

/***************************************************************************/ /**
  * \file       rtl.h
  * Definition of the classes that describe an RTL, a low-level
  *       register transfer list. Higher-level RTLs (instance
  *       of class HLJump, HLCall, etc.) represent information about
  *       a control transfer instruction (CTI) in the source program.
  *       analysis code adds information to existing higher-level
  *       RTLs and sometimes creates new higher-level RTLs (e.g., for
  *       switch statements).
  ******************************************************************************/

#ifndef __RTL_H__
#define __RTL_H__

#include <list>
#include <vector>
#include <map>
#include <set>
#include <memory>
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
class Function;
class XMLProgParser;
class StmtVisitor;
class Instruction;
class QString;

enum STMT_KIND : uint8_t;

/***************************************************************************/ /**
  * Class RTL: describes low level register transfer lists (actually lists of statements).
  * \note when time permits, this class could be removed, replaced with new Statements that mark the current native
  * address
  ******************************************************************************/
class RTL : public std::list<Instruction *> {
    ADDRESS nativeAddr; // RTL's source program instruction address
  public:
    RTL();
    RTL(ADDRESS instNativeAddr, const std::list<Instruction *> *listStmt = nullptr);
    RTL(const RTL &other); // Makes deep copy of "other"

    RTL *clone() const;
    RTL &operator=(const RTL &other);

    // Common enquiry methods
    ADDRESS getAddress() { return nativeAddr; }    //!< Return RTL's native address
    void setAddress(ADDRESS a) { nativeAddr = a; } //!< Set the address
    bool areFlagsAffected();                       //!< True if flags are affected

    // Statement list editing methods
    void appendStmt(Instruction *s); // Add s to end of RTL.
    void appendListStmt(std::list<Instruction *> &le);
    // Make a deep copy of the list of Exp*
    void deepCopyList(std::list<Instruction *> &dest) const;

    // Print RTL to a stream.
    void print(QTextStream &os, bool html = false) const;
    void dump();

    bool isCall(); // Is this RTL a call instruction?
    Instruction *getHlStmt();
    char *prints() const; // Print to a string (mainly for debugging)
  protected:
    void simplify();
    friend class XMLProgParser;
    friend class BasicBlock;
};

/***************************************************************************/ /**
  * The TableEntry class represents a single instruction - a string/RTL pair.
  *
  * This class plus ParamEntry and RTLInstDict should be moved to a separate
  * header file...
  ******************************************************************************/
class TableEntry {
  public:
    TableEntry();
    TableEntry(std::list<std::string> &p, RTL &rtl);

    const TableEntry &operator=(const TableEntry &other);

    void setParam(std::list<std::string> &p);
    void setRTL(RTL &rtl);

    // non-zero return indicates failure
    int appendRTL(std::list<std::string> &p, RTL &rtl);

  public:
    std::list<std::string> params;
    RTL rtl;

#define TEF_NEXTPC 1
    int flags; // aka required capabilities. Init. to 0
};

/***************************************************************************/ /**
  * The ParamEntry class represents the details of a single parameter.
  ******************************************************************************/
typedef enum { PARAM_SIMPLE, PARAM_ASGN, PARAM_LAMBDA, PARAM_VARIANT } ParamKind;

class ParamEntry {
  public:
    ParamEntry() {
        asgn = nullptr;
        kind = PARAM_SIMPLE;
        m_type = nullptr;
        regType = nullptr;
        lhs = false;
        mark = 0;
    }
    ~ParamEntry() {
        delete m_type;
        delete regType;
    }

    std::list<std::string> params;     //!< PARAM_VARIANT & PARAM_ASGN only */
    std::list<std::string> funcParams; //!< PARAM_LAMBDA - late bound params */
    Instruction *asgn;                   //!< PARAM_ASGN only */
    bool lhs;                          //!< True if this param ever appears on the LHS of an expression */
    ParamKind kind;
    Type *regType;        //!< Type of r[this], if any (void otherwise)
    std::set<int> regIdx; //!< Values this param can take as an r[param]
    int mark;             //!< Traversal mark. (free temporary use, basically)
  protected:
    Type *m_type;
};

/***************************************************************************/ /**
  * The RTLInstDict represents a dictionary that maps instruction names to the
  * parameters they take and a template for the Exp list describing their
  * semantics. It handles both the parsing of the SSL file that fills in
  * the dictionary entries as well as instantiation of an Exp list for a given
  * instruction name and list of actual parameters.
  ******************************************************************************/

class RTLInstDict {
  public:
    RTLInstDict();
    ~RTLInstDict();

    bool readSSLFile(const QString &SSLFileName);
    void reset();
    std::pair<std::string, unsigned> getSignature(const char *name);

    int appendToDict(const QString &n, std::list<std::string> &p, RTL &rtl);

    std::list<Instruction *> *instantiateRTL(const QString &name, ADDRESS natPC, const std::vector<Exp *> &actuals);
    std::list<Instruction *> *instantiateRTL(RTL &rtls, ADDRESS, std::list<std::string> &params,
                                           const std::vector<Exp *> &actuals);

    void transformPostVars(std::list<Instruction *> &rts, bool optimise);
    void print(QTextStream &os);
    void addRegister(const char *name, int id, int size, bool flt);
    bool partialType(Exp *exp, Type &ty);
    void fixupParams();

  public:
    //! A map from the symbolic representation of a register (e.g. "%g0") to its index within an array of registers.
    std::map<std::string, int, std::less<std::string>> RegMap;

    //! Similar to r_map but stores more info about a register such as its size, its addresss etc (see register.h).
    std::map<int, Register, std::less<int>> DetRegMap;

    //! A map from symbolic representation of a special (non-addressable) register to a Register object
    std::map<std::string, Register, std::less<std::string>> SpecialRegMap;

    //! A set of parameter names, to make sure they are declared (?).
    //! Was map from string to SemTable index
    std::set<std::string> ParamSet;

    //! Parameter (instruction operand, more like addressing mode) details (where given)
    std::map<std::string, ParamEntry> DetParamMap;

    //! The maps which summarise the semantics (.ssl) file
    std::map<std::string, Exp *> FlagFuncs;
    std::map<std::string, std::pair<int, void *> *> DefMap;
    std::map<int, Exp *> AliasMap;

    //! Map from ordinary instruction to fast pseudo instruction, for use with -f (fast but not as exact) switch
    std::map<std::string, std::string> fastMap;

    bool bigEndian; // True if this source is big endian

    //! The actual dictionary.
    std::map<QString, TableEntry, std::less<QString>> idict;

    //! An RTL describing the machine's basic fetch-execute cycle
    std::list<Instruction *> *fetchExecCycle;

    void fixupParamsSub(std::string s, std::list<std::string> &funcParams, bool &haveCount, int mark);
};

#endif /*__RTL_H__*/
