/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */
/*
 * 22 Nov 02 - Mike: Re-ordered initialisations to keep gcc quiet
 */

/*==============================================================================
 * FILE:       chllcode.h
 * OVERVIEW:   Concrete class for the "C" high level language
 *             This class is provides methods which are specific for the C
 *             language binding.  I guess this will be the most popular output
 *             language unless we do C++.
 *============================================================================*/

#ifndef _CHLLCODE_H_
#define _CHLLCODE_H_

#include <string>

class BasicBlock;
class Exp;
class Proc;

enum CTok {
	C_WHILE = 256,
	C_DO,	
	C_FOR,
	C_IF,
	C_ELSE,
	C_SWITCH,
	C_CASE,
	C_BREAK,
	C_DEFAULT,
	C_GOTO,
	C_LABEL,
	C_CALLLABEL,
	C_STATEMENT,
	C_RETURN,
	C_AND,
	C_OR,
	C_EQUAL,
	C_NOTEQUAL,
	C_LESSEQ,
	C_GTREQ,
	C_SHIFTL,
	C_SHIFTR,
	C_SYM,
	C_BADSUBSCRIPT,
	C_BADMEM,
	C_BADREG,
	C_BADSUBSCRIPTMID,
	C_BADSUBSCRIPTEND,
	C_BADMEMEND,
	C_BADREGEND,
	C_BAD,
	C_INT,
	C_INTEGER,
	C_FLOAT,
	C_STRING,
	C_FMT,
	C_NAME,
	C_IDENTIFIER,
	C_TYPE,
	C_ELLIPSIS,
};

class CHLLCode;

class CHLLToken {
protected:
	CTok tok;
	int pos;

public:
	CHLLToken(CTok t) : tok(t) { }

	CTok getTok() { return tok; }
	int getPos() { return pos; }
	void setPos(int i) { pos = i; }

	virtual void appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code);

	virtual BasicBlock *getBasicBlock() { return NULL; }
	virtual Exp *getExp() { return NULL; }
	virtual Proc *getCall() { return NULL; }
};

class CControlToken : public CHLLToken {
protected:
	BasicBlock *bb;

	virtual void appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code);

public:
	CControlToken(BasicBlock *pbb, CTok t) : CHLLToken(t), bb(pbb) { }

	virtual BasicBlock *getBasicBlock() { return bb; }
};

class CDataToken : public CHLLToken {
protected:
	Exp *exp;

	virtual void appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code);

public:	
	CDataToken(Exp *e, CTok t) : CHLLToken(t), exp(e) { }
	virtual Exp *getExp() { return exp; }
};

class CTypeToken : public CHLLToken {
protected:
	Type *type;

	virtual void appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code);

public:	
	CTypeToken(Type *type, CTok t) : CHLLToken(t), type(type) { }
	virtual Type *getType() { return type; }
};

class CProcToken : public CHLLToken {
protected:
	Proc *proc;

	virtual void appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code);

public:
	CProcToken(Proc *p, CTok t) : CHLLToken(t), proc(p) { }
	virtual Proc *getCall() { return proc; }
};

class CFmtToken : public CHLLToken {
protected:
	std::string str;

	virtual void appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code);

public:
	CFmtToken(const char *s) : CHLLToken(C_FMT), str(s) { }
virtual ~CFmtToken() { }

	void append(const char *s) { str += s; }
};

class CHLLCode : public HLLCode {
private:
	std::list<CHLLToken *> tokens;
	// current indentation level
	int indent;

	CDataToken *appendToken(Exp *exp, CTok t) { 
		CDataToken *tok = new CDataToken(exp, t);
		tokens.push_back(tok);
		return tok;
	}
	CDataToken *appendToken(Exp *exp, char ch) { 
		CDataToken *tok = new CDataToken(exp, (CTok)ch);
		tokens.push_back(tok);
		return tok;
	}
	CControlToken *appendToken(BasicBlock *pbb, CTok t) { 
		CControlToken *tok = new CControlToken(pbb, t);
		tokens.push_back(tok);
		return tok;
	}
	CProcToken *appendToken(Proc *p, CTok t) { 
		CProcToken *tok = new CProcToken(p, t);
		tokens.push_back(tok);
		return tok;
	}
	CTypeToken *appendToken(Type *type, CTok t) {
		CTypeToken *tok = new CTypeToken(type, t);
		tokens.push_back(tok);
		return tok;
	}
	CHLLToken *appendToken(char ch) { 
		CHLLToken *tok = new CHLLToken((CTok)ch);
		tokens.push_back(tok);
		return tok;
	}
	void appendExp(Exp *exp);

public:
	// constructor
	CHLLCode();
	CHLLCode(UserProc *p);

	// destructor
	virtual ~CHLLCode();

	// clear this class, calls the base
	virtual void reset();

	// formating
	void appendIndent(std::string &s);
	void appendNL(std::string &s);
	void incIndent() { indent++; }
	void decIndent() { indent--; assert(indent >= 0); }

	/*
	 * Functions to add new code
	 */

	// pretested loops (cond is optional because it is in the bb [somewhere])
	virtual void AddPretestedLoopHeader(BasicBlock *bb, Exp *cond = NULL);
	virtual void AddPretestedLoopEnd(BasicBlock *bb);

	// endless loops
	virtual void AddEndlessLoopHeader(BasicBlock *bb);
	virtual void AddEndlessLoopEnd(BasicBlock *bb);

	// posttested loops
	virtual void AddPosttestedLoopHeader(BasicBlock *bb);
	virtual void AddPosttestedLoopEnd(BasicBlock *bb, Exp *cond = NULL);

	// case conditionals "nways"
	virtual void AddCaseCondHeader(BasicBlock *bb, Exp *cond = NULL);
	virtual void AddCaseCondOption(BasicBlock *bb, Exp *opt = NULL);
	virtual void AddCaseCondOptionEnd(BasicBlock *bb);
	virtual void AddCaseCondElse(BasicBlock *bb);
	virtual void AddCaseCondEnd(BasicBlock *bb);

	// if conditions
	virtual void AddIfCondHeader(BasicBlock *bb, Exp *cond = NULL);
	virtual void AddIfCondEnd(BasicBlock *bb);

	// if else conditions
	virtual void AddIfElseCondHeader(BasicBlock *bb, Exp *cond = NULL);
	virtual void AddIfElseCondOption(BasicBlock *bb);
	virtual void AddIfElseCondEnd(BasicBlock *bb);

	// else conditions
	virtual void AddElseCondHeader(BasicBlock *bb, Exp *cond = NULL);
	virtual void AddElseCondEnd(BasicBlock *bb);

	// goto, break, continue, etc
	virtual void AddGoto(BasicBlock *bb, BasicBlock *dest = NULL);

	// labels
	virtual void AddLabel(BasicBlock *bb);

	// sequential statements
	virtual void AddAssignmentStatement(BasicBlock *bb, Exp *exp);
	virtual void AddCallStatement(BasicBlock *bb, Exp *retloc, Proc *proc, std::vector<Exp*> &args);
	virtual void AddCallStatement(BasicBlock *bb, Exp *retloc, Exp *dest, std::vector<Exp*> &args);
	virtual void AddReturnStatement(BasicBlock *bb, Exp *ret);
	virtual void AddProcStart(Signature *signature);
	virtual void AddProcEnd();
	virtual void AddLocal(const char *name, Type *type);

	/*
	 * output functions
	 */
	virtual void toString(std::string &s);
	virtual bool isLabelAt(int nCharIndex);
	virtual BasicBlock *getBlockAt(int nCharIndex);
	virtual Exp *getExpAt(int nCharIndex);
	virtual Proc *getCallAt(int nCharIndex);
	virtual void addFormating(int nCharIndex, const char *str);
};

#endif


