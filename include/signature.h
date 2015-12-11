/*
 * Copyright (C) 2002, Trent Waddington
 */
/***************************************************************************/ /**
  * \file    signature.h
  * \brief   Provides the definition for the signature classes.
  *
  *
  * \note Trent had the idea of "promoting" to signatures with known behaviour (e.g. conforms to ABI). However, it seems
  *more
  *    general to only assume ABI behaviour for library functions, and derive the signature information from child
  *    procedures in all user procedures. At present, this promotion is basically disabled (promotion always succeeds,
  *    but not much is assumed by the process of promotion). The role of the Signature classes is still being
  *considered.
  *    - MVE Jun 2005.
  */

#ifndef __SIGNATURE_H_
#define __SIGNATURE_H_

#include "operator.h"
#include "type.h"
#include "sigenum.h" // For enums platform and cc
#include "memo.h"
#include "statement.h" // For class Return

#include <string>
#include <QString>

class Instruction;
class StatementList;
class LoaderInterface;
class XMLProgParser;
class Exp;

class Parameter {
  private:
    SharedType type;
    QString m_name = "";
    Exp *exp = nullptr;
    QString boundMax;

  public:
    Parameter(SharedType _type, const QString &_name, Exp *_exp = nullptr, const QString &_boundMax = "")
        : type(_type), m_name(_name), exp(_exp), boundMax(_boundMax) {}
    virtual ~Parameter();
    bool operator==(Parameter &other);
    Parameter *clone();

    SharedType getType() { return type; }
    void setType(SharedType ty) { type = ty; }
    const QString &name() { return m_name; }
    void name(const QString &nam) { m_name = nam; }
    Exp *getExp() { return exp; }
    void setExp(Exp *e) { exp = e; }

    // this parameter is the bound of another parameter with name nam
    QString getBoundMax() { return boundMax; }
    void setBoundMax(const QString &nam);

  protected:
    friend class XMLProgParser;
    Parameter() {}
}; // class Parameter

class Return {
  public:
    SharedType type;
    Exp *exp;

    Return(SharedType _type, Exp *_exp) : type(_type), exp(_exp) {}
    virtual ~Return() {}
    bool operator==(Return &other);
    Return *clone();

    Return() : exp(nullptr) {}
    friend class XMLProgParser;
}; // class Return

typedef std::vector<Return *> Returns;

class Signature {
  protected:
    QString name;        // name of procedure
    QString sigFile; // signature file this signature was read from (for libprocs)
    std::vector<Parameter *> params;
    // std::vector<ImplicitParameter*> implicitParams;
    Returns returns;
    SharedType rettype;
    bool ellipsis;
    bool unknown;
    // bool        bFullSig;            // True if have a full signature from a signature file etc
    // True if the signature is forced with a -sf entry, or is otherwise known, e.g. WinMain
    bool forced;
    SharedType preferedReturn;
    QString preferedName;
    std::vector<int> preferedParams;

    //        void        updateParams(UserProc *p, Statement *stmt, bool checkreach = true);
    bool usesNewParam(UserProc *, Instruction *stmt, bool checkreach, int &n);

    // void        addImplicitParametersFor(Parameter *p);
    // void        addImplicitParameter(SharedType type, const char *name, Exp *e, Parameter *parent);

  public:
    Signature(const QString &nam);
    // Platform plat, calling convention cc (both enums)
    // nam is name of the procedure (no longer stored in the Proc)
    static Signature *instantiate(platform plat, callconv cc, const QString &nam);
    virtual ~Signature();

    virtual bool operator==(Signature &other);

    // clone this signature
    virtual Signature *clone();

    bool isUnknown() const { return unknown; }
    void setUnknown(bool b) { unknown = b; }
    //      void        setFullSig(bool full) {bFullSig = full;}
    bool isForced() const { return forced; }
    void setForced(bool f) { forced = f; }

    // get the return location
    virtual void addReturn(SharedType type, Exp *e = nullptr);
    virtual void addReturn(Exp *e);
    virtual void addReturn(Return *ret) { returns.push_back(ret); }
    virtual void removeReturn(Exp *e);
    virtual size_t getNumReturns() { return returns.size(); }
    virtual Exp *getReturnExp(size_t n) { return returns[n]->exp; }
    void setReturnExp(size_t n, Exp *e) { returns[n]->exp = e; }
    virtual SharedType getReturnType(size_t n) { return returns[n]->type; }
    virtual void setReturnType(size_t n, SharedType ty);
    int findReturn(Exp *e);
    //      void        fixReturnsWithParameters();            // Needs description
    void setRetType(SharedType t) { rettype = t; }
    Returns &getReturns() { return returns; }
    SharedType getTypeFor(Exp *e);

    // get/set the name
    virtual QString getName();
    virtual void setName(const QString &nam);
    // get/set the signature file
    const QString &getSigFile() const { return sigFile; }
    void setSigFile(const QString &nam) { sigFile = nam; }

    // add a new parameter to this signature
    virtual void addParameter(const char *nam = nullptr);
    virtual void addParameter(SharedType type, const QString &nam = QString::null, Exp *e = nullptr,
                              const QString &boundMax = "");
    virtual void addParameter(Exp *e, SharedType ty);
    virtual void addParameter(Parameter *param);
    void addEllipsis() { ellipsis = true; }
    void killEllipsis() { ellipsis = false; }
    virtual void removeParameter(Exp *e);
    virtual void removeParameter(size_t i);
    // set the number of parameters using defaults
    virtual void setNumParams(size_t n);

    // accessors for parameters
    virtual size_t getNumParams() { return params.size(); }
    virtual const QString &getParamName(size_t n);
    virtual Exp *getParamExp(int n);
    virtual SharedType getParamType(int n);
    virtual QString getParamBoundMax(int n);
    virtual void setParamType(int n, SharedType ty);
    virtual void setParamType(const char *nam, SharedType ty);
    virtual void setParamType(Exp *e, SharedType ty);
    virtual void setParamName(int n, const char *nam);
    virtual void setParamExp(int n, Exp *e);
    virtual int findParam(Exp *e);
    virtual int findParam(const QString &nam);
    // accessor for argument expressions
    virtual Exp *getArgumentExp(int n);
    virtual bool hasEllipsis() { return ellipsis; }

    void renameParam(const QString &oldName, const char *newName);

    // analysis determines parameters / return type
    // virtual void analyse(UserProc *p);
    bool dfaTypeAnalysis(Cfg *cfg);
    virtual Signature *promote(UserProc *p);
    void print(QTextStream &out, bool = false) const;
    char *prints(); // For debugging
    void printToLog();

    // Special for Mike: find the location that conventionally holds the first outgoing (actual) parameter
    // MVE: Use the below now
    Exp *getFirstArgLoc(Prog *prog);

    // This is like getParamLoc, except that it works before Signature::analyse is called.  It is used only to order
    // parameters correctly, for the common case where the proc will end up using a standard calling convention
    Exp *getEarlyParamExp(int n, Prog *prog);

    // Get a wildcard to find stack locations
    virtual Exp *getStackWildcard() { return nullptr; }
    class StackRegisterNotDefinedException : public std::exception {
      public:
        StackRegisterNotDefinedException() {}
    };
    virtual int getStackRegister() noexcept(false);
    static int getStackRegister(Prog *prog) noexcept(false);
    bool isStackLocal(Prog *prog, Exp *e);
    // Similar to the above, but checks for address of a local (i.e. sp{0} -/+ K)
    virtual bool isAddrOfStackLocal(Prog *prog, Exp *e);
    // For most machines, local variables are always NEGATIVE offsets from sp
    virtual bool isLocalOffsetNegative() { return true; }
    // For most machines, local variables are not POSITIVE offsets from sp
    virtual bool isLocalOffsetPositive() { return false; }
    // Is this operator (between the stack pointer and a constant) compatible with a stack local pattern?
    bool isOpCompatStackLocal(OPER op);

    // Quick and dirty hack
    static Exp *getReturnExp2(LoaderInterface *pBF);
    static StatementList &getStdRetStmt(Prog *prog);

    // get anything that can be proven as a result of the signature
    virtual Exp *getProven(Exp * /*left*/) { return nullptr; }
    virtual bool isPreserved(Exp * /*e*/) { return false; }     // Return whether e is preserved by this proc
    virtual void setLibraryDefines(StatementList * /*defs*/) {} // Set the locations defined by library calls
    static void setABIdefines(Prog *prog, StatementList *defs);

    // Return true if this is a known machine (e.g. SparcSignature as opposed to Signature)
    virtual bool isPromoted() { return false; }
    // Return true if this has a full blown signature, e.g. main/WinMain etc.
    // Note that many calls to isFullSignature were incorrectly calls to isPromoted()
    // bool        isFullSignature() {return bFullSig;}

    // ascii versions of platform, calling convention name
    static QString platformName(platform plat);
    static QString conventionName(callconv cc);
    virtual platform getPlatform() { return PLAT_GENERIC; }
    virtual callconv getConvention() { return CONV_NONE; }

    // prefered format
    void setPreferedReturn(SharedType ty) { preferedReturn = ty; }
    void setPreferedName(const QString &nam) { preferedName = nam; }
    void addPreferedParameter(int n) { preferedParams.push_back(n); }
    SharedType getPreferedReturn() { return preferedReturn; }
    const QString &getPreferedName() { return preferedName; }
    size_t getNumPreferedParams() { return preferedParams.size(); }
    int getPreferedParam(size_t n) { return preferedParams[n]; }

    // A compare function for arguments and returns. Used for sorting returns in calcReturn() etc
    virtual bool argumentCompare(Assignment &a, Assignment &b);
    virtual bool returnCompare(Assignment &a, Assignment &b);

    bool isNoReturn() const { return false; }
  protected:
    friend class XMLProgParser;
    Signature() : name(""), rettype(nullptr), ellipsis(false), preferedReturn(nullptr), preferedName("") {}
    void appendParameter(Parameter *p) { params.push_back(p); }
    // void        appendImplicitParameter(ImplicitParameter *p) { implicitParams.push_back(p); }
    void appendReturn(Return *r) { returns.push_back(r); }
}; // class Signature

class CustomSignature : public Signature {
  protected:
    int sp;

  public:
    CustomSignature(const QString &nam);
    virtual ~CustomSignature() {}
    virtual bool isPromoted() override { return true; }
    virtual Signature *clone() override;
    void setSP(int nsp);
    virtual int getStackRegister() noexcept(false) override { return sp; }
};

#endif
