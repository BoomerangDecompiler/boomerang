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

#pragma once

#include "db/register.h"        // for Register
#include "type/type.h"          // for Type
#include "include/types.h"      // for ADDRESS

#include <functional>           // for less
#include <list>                 // for list
#include <map>                  // for map
#include <set>                  // for set
#include <string>               // for string
#include <utility>              // for pair
#include <vector>               // for vector
#include <QMap>
#include <memory>

class Exp;         // lines 38-38
class Instruction; // lines 47-47
class QTextStream;
class QString;

enum StmtType : uint8_t;
using SharedRTL = std::shared_ptr<class RTL>;
using SharedExp = std::shared_ptr<Exp>;


/***************************************************************************/ /**
 * Class RTL: describes low level register transfer lists (actually lists of statements).
 *
 * \note when time permits, this class could be removed,
 * replaced with new Statements that mark the current native address
 ******************************************************************************/
class RTL : public std::list<Instruction *>
{
	friend class XMLProgParser;
	friend class BasicBlock;

private:
	ADDRESS m_nativeAddr; ///< RTL's source program instruction address

public:
	RTL();
	RTL(ADDRESS instNativeAddr, const std::list<Instruction *> *listStmt = nullptr);

	/***************************************************************************/ /**
	 * \brief        Copy constructor. A deep clone is made of the given object
	 *                    so that the lists of Exps do not share memory.
	 * \param        other RTL to copy from
	 ******************************************************************************/
	RTL(const RTL& other);
	~RTL();

	/***************************************************************************/ /**
	 * \brief        Deep copy clone; deleting the clone will not affect this
	 *               RTL object
	 * \returns      Pointer to a new RTL that is a clone of this one
	 ******************************************************************************/
	RTL *clone() const;

	/***************************************************************************/ /**
	 * \brief        Assignment copy (deep).
	 * \param        other - RTL to copy
	 * \returns      a reference to this object
	 ******************************************************************************/
	RTL& operator=(const RTL& other);

	/// Common enquiry methods
	ADDRESS getAddress() const { return m_nativeAddr; }    ///< Return RTL's native address
	void setAddress(ADDRESS a) { m_nativeAddr = a; } ///< Set the address

	// Statement list editing methods

	/***************************************************************************/ /**
	 * \brief        Append the given Statement at the end of this RTL
	 * \note         Exception: Leaves any flag call at the end (so may push exp
	 *               to second last position, instead of last)
	 * \note         stmt is NOT copied. This is different to how UQBT was!
	 * \param        s pointer to Statement to append
	 ******************************************************************************/
	void appendStmt(Instruction *s); // Add s to end of RTL.

	/***************************************************************************/ /**
	 * \brief  Append a given list of Statements to this RTL
	 * \note   A copy of the Statements in le are appended
	 * \param  le - list of Statements to insert
	 ******************************************************************************/
	void appendListStmt(std::list<Instruction *>& le);

	/***************************************************************************/ /**
	 * \brief        Make a copy of this RTLs list of Exp* to the given list
	 * \param        dest Ref to empty list to copy to
	 ******************************************************************************/
	void deepCopyList(std::list<Instruction *>& dest) const;

	/***************************************************************************/ /**
	 * \brief   Prints this object to a stream in text form.
	 * \param   os - stream to output to (often cout or cerr)
	 * \param   html - if true output is in html
	 ******************************************************************************/
	void print(QTextStream& os, bool html = false) const;
	void dump() const;

	/// Is this RTL a call instruction?
	bool isCall() const;


	/// Use this slow function when you can't be sure that the HL Statement is last
	/// Get the "special" (High Level) Statement this RTL (else nullptr)
	Instruction *getHlStmt() const;
	char *prints() const; // Print to a string (mainly for debugging)

protected:
	void simplify();
};


/***************************************************************************/ /**
 * The TableEntry class represents a single instruction - a string/RTL pair.
 *
 * \todo This class plus ParamEntry and RTLInstDict should be moved to a separate
 * header file...
 ******************************************************************************/
class TableEntry
{
public:
	TableEntry();
	TableEntry(std::list<QString>& p, RTL& rtl);

	TableEntry& operator=(const TableEntry& other);

	void setParam(std::list<QString>& p);
	void setRTL(RTL& rtl);

	// non-zero return indicates failure
	int appendRTL(std::list<QString>& p, RTL& rtl);

public:
	std::list<QString> m_params;
	RTL m_rtl;

	int m_flags; // aka required capabilities. Init. to 0
};


typedef enum
{
	PARAM_SIMPLE,
	PARAM_ASGN,
	PARAM_LAMBDA,
	PARAM_VARIANT
} ParamKind;


/***************************************************************************/ /**
 * The ParamEntry class represents the details of a single parameter.
 ******************************************************************************/
class ParamEntry
{
public:
	std::list<QString> m_params;          ///< PARAM_VARIANT & PARAM_ASGN only */
	std::list<QString> m_funcParams;      ///< PARAM_LAMBDA - late bound params */
	Instruction *m_asgn = nullptr;        ///< PARAM_ASGN only */
	bool m_lhs          = false;          ///< True if this param ever appears on the LHS of an expression */
	ParamKind m_kind    = PARAM_SIMPLE;
	SharedType m_regType;                 ///< Type of r[this], if any (void otherwise)
	std::set<int> m_regIdx;               ///< Values this param can take as an r[param]
	int m_mark = 0;                       ///< Traversal mark. (free temporary use, basically)

protected:
	SharedType m_type;
};


/***************************************************************************/ /**
 * The RTLInstDict represents a dictionary that maps instruction names to the
 * parameters they take and a template for the Exp list describing their
 * semantics. It handles both the parsing of the SSL file that fills in
 * the dictionary entries as well as instantiation of an Exp list for a given
 * instruction name and list of actual parameters.
 ******************************************************************************/
class RTLInstDict
{
public:
	RTLInstDict();
	~RTLInstDict();

	bool readSSLFile(const QString& SSLFileName);
	void reset();

	std::pair<QString, unsigned> getSignature(const char *name);

	int appendToDict(const QString& n, std::list<QString>& p, RTL& rtl);

	std::list<Instruction *> *instantiateRTL(const QString& name, ADDRESS natPC, const std::vector<SharedExp>& actuals);

	std::list<Instruction *> *instantiateRTL(RTL & rtls, ADDRESS, std::list<QString> &params,
											 const std::vector<SharedExp> &actuals);

	void transformPostVars(std::list<Instruction *>& rts, bool optimise);
	void print(QTextStream& os);
	void addRegister(const QString& name, int id, int size, bool flt);
	bool partialType(Exp *exp, Type& ty);
	void fixupParams();

public:
	/// An RTL describing the machine's basic fetch-execute cycle
	SharedRTL fetchExecCycle;

	/// A map from the symbolic representation of a register (e.g. "%g0") to its index within an array of registers.
	std::map<QString, int, std::less<QString> > RegMap;

	/// Similar to r_map but stores more info about a register such as its size, its addresss etc (see register.h).
	std::map<int, Register, std::less<int> > DetRegMap;


	/// A set of parameter names, to make sure they are declared (?).
	/// Was map from string to SemTable index
	std::set<QString> ParamSet;

	/// Parameter (instruction operand, more like addressing mode) details (where given)
	QMap<QString, ParamEntry> DetParamMap;

	/// The maps which summarise the semantics (.ssl) file
	std::map<QString, SharedExp> FlagFuncs;
	/// Map from ordinary instruction to fast pseudo instruction, for use with -f (fast but not as exact) switch
	std::map<QString, QString> fastMap;

	bool m_bigEndian; // True if this source is big endian

private:
	void fixupParamsSub(const QString& s, std::list<QString>& funcParams, bool& haveCount, int mark);

	/// A map from symbolic representation of a special (non-addressable) register to a Register object
	std::map<QString, Register, std::less<QString> > SpecialRegMap;

	std::map<QString, std::pair<int, void *> *> DefMap;

	std::map<int, SharedExp> AliasMap;

	/// The actual dictionary.
	std::map<QString, TableEntry, std::less<QString> > idict;
};
