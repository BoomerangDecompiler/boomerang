#pragma once

#include "boomerang/util/Types.h"
#include "boomerang/db/Proc.h"
#include "boomerang/db/exp/Exp.h"


/***************************************************************************/ /**
* Const is a subclass of Exp, and holds either an integer, floating point, string, or address constant
******************************************************************************/
class Const : public Exp
{
private:
    union
    {
        int      i;    ///< Integer
        QWord    ll;   ///< 64 bit integer / address / pointer
        double   d;    ///< Double precision float

        /// Don't store string: function could be renamed
        Function *pp;      ///< Pointer to function
    } u;

    QString m_string;
    int m_conscript;   ///< like a subscript for constants
    SharedType m_type; ///< Constants need types during type analysis

public:
    // Special constructors overloaded for the various constants
    Const(uint32_t i);
    Const(int i);
    Const(QWord ll);

    /// \remark This is bad. We need a way of constructing true unsigned constants
    Const(Address a);
    Const(double d);
    //    Const(const char *p);
    Const(const QString& p);
    Const(Function *p);
    // Copy constructor
    Const(const Const& o);

    template<class T>
    static std::shared_ptr<Const> get(T i) { return std::make_shared<Const>(i); }

    /// Nothing to destruct: Don't deallocate the string passed to constructor
    virtual ~Const() {}

    // Clone
    virtual SharedExp clone() const override;

    // Compare

    /***************************************************************************/ /**
    * \brief        Virtual function to compare myself for equality with
    *               another Exp
    * \param  o     Ref to other Exp
    * \returns      True if equal
    ******************************************************************************/
    virtual bool operator==(const Exp& o) const override;

    /***************************************************************************/ /**
    * \brief      Virtual function to compare myself with another Exp
    * \note       The test for a wildcard is only with this object, not the other object (o).
    *             So when searching and there could be wildcards, use search == *this not *this == search
    * \param      o - Ref to other Exp
    * \returns    true if equal
    ******************************************************************************/
    virtual bool operator<(const Exp& o) const override;

    /***************************************************************************/ /**
    * \brief        Virtual function to compare myself for equality with another Exp, *ignoring subscripts*
    * \param        o - Ref to other Exp
    * \returns      True if equal
    ******************************************************************************/
    virtual bool operator*=(const Exp& o) const override;

    // Get the constant
    int getInt() const { return u.i; }
    QWord getLong() const { return u.ll; }
    double getFlt() const { return u.d; }
    QString getStr() const { return m_string; }
    Address getAddr() const { return Address((Address::value_type)u.ll); }
    QString getFuncName() const;

    // Set the constant
    void setInt(int i) { u.i = i; }
    void setLong(QWord ll) { u.ll = ll; }
    void setFlt(double d) { u.d = d; }
    void setStr(const QString& p) { m_string = p; }
    void setAddr(Address a) { u.ll = a.value(); }

    // Get and set the type
    SharedType getType() { return m_type; }
    const SharedType getType() const { return m_type; }
    void setType(SharedType ty) { m_type = ty; }

    /***************************************************************************/ /**
    * \brief       "Print" in infix notation the expression to a stream.
    *              Mainly for debugging, or maybe some low level windows
    * \param       os  - Ref to an output stream
    ******************************************************************************/
    virtual void print(QTextStream& os, bool = false) const override;

    /// Print "recursive" (extra parens not wanted at outer levels)
    void printNoQuotes(QTextStream& os) const;
    virtual void printx(int ind) const override;

    virtual void appendDotFile(QTextStream& of) override;
    virtual SharedExp genConstraints(SharedExp restrictTo) override;

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual SharedExp accept(ExpModifier *v) override;

    /***************************************************************************/ /**
    * \brief        Matches this expression to the given patten
    * \param        pattern to match
    * \returns            list of variable bindings, or nullptr if matching fails
    ******************************************************************************/
    virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;

    int getConscript() const { return m_conscript; }
    void setConscript(int cs) { m_conscript = cs; }

    virtual SharedType ascendType() override;
    virtual void descendType(SharedType parentType, bool& ch, Instruction *s) override;
};
