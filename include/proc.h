/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       proc.h
 * OVERVIEW:   Interface for the procedure classes, which are used to
 *             store information about variables in the procedure such
 *             as parameters and locals.
 *============================================================================*/

/* $Revision$
 * 11 Mar 98 - Cristina  
 *  replaced BOOL for bool type (C++'s), same for TRUE and FALSE.
 * 18 Mar 98 - Cristina 
 *  added procedure iterator typedef (PROC_IT), and print() function.
 *  added GetCFG() function and changed the member Cfg to be a non pointer.
 * 24 Jul 98 - Mike
 *  Directly #include ../CFG/cfg.h now
 * 15 Dec 98 - Mike: Coverage functions
 * 16 Dec 98 - Mike: GetCFG() returns a pointer now
 * 08 Jan 99 - Mike: cfg is a pointer now, so can delete the whole CFG
 * 27 Jan 99 - Mike: Added m_bDecoded (in place of m_bIncomplete)
 * 19 Jan 99 - Mike: Removed path to "cfg.h"
 * 07 Apr 99 - Mike: Mods for HEADERS
 * 27 Apr 99 - Mike: Mods to some parameter stuff; added SetLibParams()
 * 29 Apr 99 - Mike: Added m_bDecoded and m_bDecoding, etc
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 15 Mar 00 - Cristina: UserProc::setAFP transformed to setAXP
 * 29 Mar 00 - Mike: Removed Type class
 * 14 Jun 00 - Mike: Changes for new Coverage system
 * 21 Jun 00 - Mike: Removed setTailCaller
 * 30 Aug 00 - Sameer/Brian: Added UserProc::getFirstLocalIndex() and
 *              UserProc::getLastLocalIndex() to return the index of the first
 *              and last symbolic local of each procedure.
 * 15 Sep 00 - Mike: matchParams takes a ReturnLocations parameter now
 * 18 Sep 00 - Mike: Removed the last parametrer to matchParams; outgoing is
 *              a reference now (was pointer)
 *              setReturnType() functions now return a bool
 * 20 Nov 00 - Mike: Added getVarType()
 * 19 Dec 00 - Mike: Added checkReturnPass() and checkReturnPassBB()
 * 21 Dec 00 - Mike: Removed replaceParentStackAddresses()
 * 23 Feb 01 - Mike: Added checkMemSizes and findVarEntry for "ninths" test.
 *              Also, symbolMap is totally type insensitive now
 * 13 Aug 01 - Bernard: Added support for type analysis
 * 30 Aug 01 - Mike: Proc's parameters changed from vector to list
 *              Also, findVarEntry returns SemStr* now.
 * 20 Sep 01 - Brian: Added getSymbolicLocals() to return the list of symbolic
 *              locals for a procedure.
*/

#ifndef _PROC_H_
#define _PROC_H_

#include <list>
#include <vector>
#include <map>
#include <set>
#include <string>
#include "coverage.h"           // For Coverage class

class Prog;
class UserProc;
class Cfg;
class BasicBlock;
typedef BasicBlock* PBB;
class Exp;
class TypedExp;
class lessTI;
class Type;
class RTL;
class HLLCode;
class HLCall;
class Parameter;
class Argument;
class Signature;

/*==============================================================================
 * Procedure class.
 *============================================================================*/
class Proc {
public:

    /*
     * Constructor with name, native address and optional bLib.
     */
    Proc(Prog *prog, ADDRESS uNative, Signature *sig);

    virtual ~Proc();

    /*
     * Gets name of this procedure.
     */
    const char* getName();

    /*
     * Gets sets the name of this procedure.
     */
    void setName(const char *nam);

    /*
     * Get the native address.
     */
    ADDRESS getNativeAddress();

	/*
	 * Set the native address
	 */
	void setNativeAddress(ADDRESS a);

	/*
	 * Get the program this procedure belongs to.
	 */
	Prog *getProg();

	/*
	 * Get/Set the first procedure that calls this procedure (or null for main/start).
	 */
	Proc *getFirstCaller();
	void setFirstCaller(Proc *p) { if (m_firstCaller == NULL) m_firstCaller = p; }

	/*
	 * Returns a poiner to the Signature
	 */
	Signature *getSignature();

    /*
     * Prints this procedure to an output stream.
     */
    virtual std::ostream& put(std::ostream& os) = 0;

    /*
     * Return the coverage of this procedure in bytes.
     */
//    virtual unsigned getCoverage() = 0;

    /*
     * Modify actuals so that it is now the list of locations that must
     * be passed to this procedure. The modification will be to either add
     * dummp locations to actuals, delete from actuals, or leave it
     * unchanged.
     * Add "dummy" params: this will be required when there are
     *   less live outs at a call site than the number of parameters
     *   expected by the procedure called. This will be a result of
     *   one of two things:
     *   i) a value returned by a preceeding call is used as a
     *      parameter and as such is not detected as defined by the
     *      procedure. E.g.:
     *
     *         foo(bar(x));
     *
     *      Here, the result of bar(x) is used as the first and only
     *      parameter to foo. On some architectures (such as SPARC),
     *      the location used as the first parameter (e.g. %o0) is
     *      also the location in which a value is returned. So, the
     *      call to bar defines this location implicitly as shown in
     *      the following SPARC assembly that may be generated by from
     *      the above code:
     *
     *          mov   x, %o0
     *          call  bar
     *          nop
     *          call  foo
     *
     *     As can be seen, there is no definition of %o0 after the
     *     call to bar and before the call to foo. Adding the integer
     *     return location is therefore a good guess for the dummy
     *     location to add (but may occasionally be wrong).
     *
     *  ii) uninitialised variables are used as parameters to a call
     *
     *  Note that both of these situations can only occur on
     *  architectures such as SPARC that use registers for parameter
     *  passing. Stack parameters must always be pushed so that the
     *  callee doesn't access the caller's non-parameter portion of
     *  stack.
     *
     * This used to be a virtual function, implemented differenty for
     * LibProcs and for UserProcs. But in fact, both need the exact same
     * treatment; the only difference is how the local member "parameters"
     * is set (from common.hs in the case of LibProc objects, or from analysis
     * in the case of UserProcs).
     */
    void matchParams(std::list<Exp*>& actuals, UserProc& caller);

    /*
     * Get a list of types to cast a given list of actual parameters to
     */
    std::list<Type>* Proc::getParamTypeList(const std::list<Exp*>& actuals);

	/*
	 * Set the number of bytes popped off the caller stack by this procedure
	 */
	void setBytesPopped(int n);
	int getBytesPopped() { return bytesPopped; }

    /*
     * Return true if this is a library proc
     */
    virtual bool isLib() {return false;}

    /*
     * Return true if the aggregate pointer is used.
     * It is assumed that this is false for library procs
     */
    virtual bool isAggregateUsed() {return false;}

    /*
     * OutPut operator for a Proc object.
     */
    friend std::ostream& operator<<(std::ostream& os, Proc& proc);
    
    /*
     * Stores a list of reg to the procedure used for 
     * type propagation
     */
    std::list<int> regParams;

	// serialize this procedure
	virtual bool serialize(std::ostream &ouf, int &len) = 0;

	// deserialize a procedure
	static Proc *deserialize(Prog *prog, std::istream &inf);
	virtual bool deserialize_fid(std::istream &inf, int fid);

protected:

    /*
     * Program containing this procedure.
     */
    Prog *prog;

    /*
     * Procedure's address.
     */
    ADDRESS address;

    /*
     * The formal signature of this procedure. This information is determined
     * either by the common.hs file (for a library function) or by analysis.
     */
    Signature *signature;

	/*
	 * The first procedure to call this procedure
	 */
	Proc *m_firstCaller;
	ADDRESS m_firstCallerAddr;  // can only be used once.

	/*
	 * Number of bytes this procedure will cause any call to it to pop off
	 * the stack (of the caller).
	 */
	int bytesPopped;


}; 

/*==============================================================================
 * LibProc class.
 *============================================================================*/
class LibProc : public Proc {
public:
	
    LibProc(Prog *prog, std::string& name, ADDRESS address);
	virtual ~LibProc();

    /*
     * Return the coverage of this procedure in bytes.
     */
    unsigned getCoverage() { return 0; }

#if 0
    /*
     * See comment for Proc::matchParams.
     */
    void matchParams(std::list<Exp*>& actuals, UserProc& caller,
        const Parameters* outgoing, const Exp** intRetLoc) const;
#endif

    /*
     * Return true, since is a library proc
     */
    bool isLib() {return true;}

    /*
     * Return true if the aggregate pointer is used.
     * It is assumed that this is false for library procs
     */
    virtual bool isAggregateUsed() {return false;}

    /*
     * Prints this procedure to an output stream.
     */
    std::ostream& put(std::ostream& os);

	// serialize this procedure
	virtual bool serialize(std::ostream &ouf, int &len);
	// deserialize the subclass specific portion of this procedure
	virtual bool deserialize_fid(std::istream &inf, int fid);

};

/*==============================================================================
 * UserProc class.
 *============================================================================*/
class UserProc : public Proc {
public:

    UserProc(Prog *prog, std::string& name, ADDRESS address);
	virtual ~UserProc();

    /*
     * Records that this procedure has been decoded.
     */
    void setDecoded();

	/*
	 * Removes the decoded bit and throws away all the current information 
	 * about this procedure.
	 */
	void unDecode();

    /*
     * Returns a pointer to the CFG.
     */
    Cfg* getCFG();

    /*
     * Deletes the whole CFG and all the RTLs, RTs, and Exp*s associated with
     * it. Also nulls the internal cfg pointer (to prevent strange errors)
     */
    void deleteCFG();

    /*
     * Returns whether or not this procedure can be decoded (i.e. has
     * it already been decoded).
     */
    bool isDecoded();

    /*
     * Return the number of bytes allocated for locals on the stack.
     */
    int getLocalsSize();

    /*
     * Get the type of the given var
     */
//    Type getVarType(int idx);

    /*
     * Set the size of the given var
     */
//    void setVarSize(int idx, int size);

	// serialize this procedure
	virtual bool serialize(std::ostream &ouf, int &len);
	// deserialize the subclass specific portion of this procedure
	virtual bool deserialize_fid(std::istream &inf, int fid);

	// code generation
	bool generateCode(HLLCode &hll);

        // print this proc, mainly for debugging
        void print(std::ostream &out);

	// decompile this proc
	void decompile();
	bool removeNullStatements();
	bool removeDeadStatements();
        bool propogateAndRemoveStatements();

	// promote the signature if possible
	void promoteSignature();

	// get all the statements
	void getAllStatements(std::set<Statement*> &stmts);

	// remove a statement
	void removeStatement(Statement *stmt);

private:
    /*
     * Find a pointer to the Exp* representing the given var
     * Used by the above 2
     */
    Exp** findVarEntry(int idx);

    /*
     * A special pass to check the sizes of memory that is about to be converted
     * into a var, ensuring that the largest size used in the proc is used for
     * all references (and it's declared that size)
     */
    void    checkMemSizes();

    /*
     * Implement the above for one given Exp*
     */
    void    checkMemSize(Exp* e);

public:

    /*
     * Return the coverage of this procedure in bytes.
     */
//    unsigned getCoverage() {return cover.totalCover();}

    /*
     * Sets the parameters that have been recovered for this procedure through
     * analysis.
     */
    void setParams(std::list<TypedExp*>& params, bool aggUsed = false);

    /*
     * Given a machine dependent location, return a generated symbolic
     * representation for it.
     */
    void toSymbolic(TypedExp* loc, TypedExp* result, bool local = true);

    /*
     * Return the next available local variable; make it the given type
     */
    TypedExp* newLocal(Type* ty);

    /*
     * Print the locals declaration in C style.
     */
    void printLocalsAsC(std::ostream& os);

    /*
     * Return the index of the first symbolic local for the procedure.
     */
    int getFirstLocalIndex();

    /*
     * Return the index of the last symbolic local for the procedure.
     */
    int getLastLocalIndex();
    
    /*
     * Return the list of symbolic locals for the procedure.
     */
    std::vector<TypedExp*>& getSymbolicLocals();
    
    /*
     * Replace each instance of a location in this procedure with its symbolic
     * representation if it has one.
     */
    void propagateSymbolics();

    /*
     * Get the BB that is the entry point (not always the first BB)
     */
    PBB getEntryBB();

    /*
     * Prints this procedure to an output stream.
     */
    std::ostream& put(std::ostream& os);

    /*
     * Set the entry BB for this procedure (constructor has the entry address)
     */
    void setEntryBB();

    /*
     * Get the callees
     */
    std::set<Proc*>& getCallees();

    /*
     * Add to the set of callees
     */
    void setCallee(Proc* callee); 

	/*
	 * return true if this procedure contains the given address
	 */
	bool containsAddr(ADDRESS uAddr);

    /*
     * Add (st, fi) to the set of ranges covered in this procedure
     */
//    void addRange(ADDRESS st, ADDRESS fi) {cover.addRange(st, fi);}

    /*
     * Add all the ranges in other to the set of ranges covered this procedure
     */
//    void addRanges(Coverage& other) {cover.addRanges(other);}

    /*
     * Print the coverage for this procedure
     */
//    void printCoverage(std::ostream& os = cout) {cover.print(os); }

    /*
     * Get the first gap (between ranges) for this Coverage object
     */
//    bool    getFirstGap(ADDRESS& a1, ADDRESS& a2, COV_CIT& it)
//                {return cover.getFirstGap(a1, a2, it);}

    /*
     * Get the next gap (between ranges) for this Coverage object
     */
//   bool    getNextGap(ADDRESS& a1, ADDRESS& a2, COV_CIT& it)
//                {return cover.getNextGap(a1, a2, it);}

    /*
     * Add this proc's range to the program's coverage
     */
//    void    addProcCoverage();

    /*
     * Check if this return location is "passed through" this function to one
     * of its callees. For example in the returncallee test, main uses the
     * return value from add4, and this use is "passed on" to add2, since
     * add4 doesn't define the return location after the call to add4
     */
    void checkReturnPass(int returnLocBit, TypedExp* returnLoc);

    /*
     * Do the main work for the above
     */
    void checkReturnPassBB(PBB pBB, HLCall* pCall, int returnLocBit,
        TypedExp* returnLoc, std::set<PBB>& seen);

    /*
     * Return true if this proc uses the special aggregate pointer as the
     * first parameter
     */
    virtual bool isAggregateUsed() {return aggregateUsed;}

	// map for local symbols
	std::map<std::string, TypedExp *> symbols;
	
	// search for a symbol which matches an expression (locals, then globals searched)
	bool findSymbolFor(Exp *e, std::string &sym, TypedExp* &sym_exp);

private:

    /*
     * The control flow graph.
     */
    Cfg* cfg;

    /*
     * True if this procedure has been decoded.
     */
    bool decoded;

    /*
     * Indicates whether or not a non-default return type has been
     * determined for this procedure.
     */
    bool returnIsSet;

    /*
     * Indicate that the procedure has had its variables converted to
     * symbolic form, e.g. r[8]->v2. This is only done once, by a call to
     * propagateSymbolics(). We need to know that this has happened if we
     * later determine a different return location, and it happens not to
     * have been converted to symbolic as yet 
     */
    bool isSymbolic;

    /*
     * Used to generate unique IDs for the parameters and locals to
     * calls that are recovered and given a symbolic name.
     */
    unsigned uniqueID;

    /*
     * Indicate that the aggregate location pointer "hidden" parameter is used,
     * and is thus explicit in this translation. Needed only by architectures
     * like Sparc where a special parent stack location is used to pass the
     * address of aggregates. Set with the setParams() member function
     */
    bool aggregateUsed;

    /*
     * This map records the allocation of local variables and their types.
     */
    std::vector<TypedExp*> locals;

    /*
     * A map between machine dependent locations and their corresponding
     * symbolic, machine independent representations.
     */
    std::map<Exp*,Exp*> symbolMap;

    /* 
     * An object that represents a set of ranges, which gives the coverage
     * of the source procedure
     */
    Coverage cover;

    /*
     * The return location as written to the .c file. Not valid unless the file
     * has been written (fileWritten true)
     */
    Exp* fileRetLocn;

    /*
     * Set of callees (Procedures that this procedure calls). Used for
     * call graph, among other things
     */
    std::set<Proc*> calleeSet;
	std::set<ADDRESS> calleeAddrSet;  // used in serialization

};      /* UserProc */
#endif
