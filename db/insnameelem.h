/*
 * Copyright (C) 2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*
 * insnameelem.h
 *
 * header for insnameelem.cc
 * class declarations for class InsNameElem
 *
 * Last Modified
 * 19 Feb 01 - Simon: created
 */

#ifndef INSNAMEELEM_H
#define INSNAMEELEM_H

#include "table.h"

class InsNameElem {

public:
	InsNameElem(const char *name);
	virtual ~InsNameElem(void);
	virtual int ntokens(void);
	virtual std::string getinstruction(void);
	virtual std::string getinspattern(void);
	virtual void getrefmap(std::map<std::string, InsNameElem*> &m);
 
	int ninstructions(void);
	void append(InsNameElem* next);
	bool increment(void);
	void reset(void);
	int getvalue(void);
	
protected:
	InsNameElem* nextelem;
	std::string elemname;
	int value;
};

class InsOptionElem : public InsNameElem {

public:
	InsOptionElem(const char *name);
	virtual int ntokens(void);
	virtual std::string getinstruction(void);
	virtual std::string getinspattern(void);
 
};

class InsListElem : public InsNameElem {

public:
	InsListElem(const char *name, Table* t, const char *idx);
	virtual int ntokens(void);
	virtual std::string getinstruction(void);
	virtual std::string getinspattern(void);
	virtual void getrefmap(std::map<std::string, InsNameElem*> &m);

	std::string getindex(void);

protected:
	std::string indexname;
	Table* thetable;
};

#endif
