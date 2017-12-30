#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/codegen/ICodeGenerator.h"
#include "boomerang/util/Address.h"

#include <string>
#include <sstream>
#include <unordered_set>

class BasicBlock;
class Exp;
class Function;
class Assign;
class LocationSet;
class IBinaryImage;


// Operator precedence

/*
 * Operator Name                Associativity    Operators
 * Primary scope resolution     left to right    ::
 * Primary                      left to right    ()    [ ]     .    -> dynamic_cast typeid
 * Unary                        right to left    ++    --    +  -  !     ~    &  *  (type_name)  sizeof new
 * delete
 * C++ Pointer to Member        left to right    .* ->*
 * Multiplicative               left to right    *  /  %
 * Additive                     left to right    +  -
 * Bitwise Shift                left to right    <<    >>
 * Relational                   left to right    <  >  <=  >=
 * Equality                     left to right    ==    !=
 * Bitwise AND                  left to right    &
 * Bitwise Exclusive OR         left to right    ^
 * Bitwise Inclusive OR         left to right    |
 * Logical AND                  left to right    &&
 * Logical OR                   left to right    ||
 * Conditional                  right to left    ? :
 * Assignment                   right to left    =  +=  -=  *=    /=    <<=     >>=  %=   &=    ^=    |=
 * Comma                        left to right    ,
 */

/// Operator precedence
enum PREC
{
    PREC_NONE = 0,  ///< Outer level (no parens required)
    PREC_COMMA,     ///< Comma
    PREC_ASSIGN,    ///< Assignment
    PREC_COND,      ///< Conditional
    PREC_LOG_OR,    ///< Logical OR
    PREC_LOG_AND,   ///< Logical AND
    PREC_BIT_IOR,   ///< Bitwise Inclusive OR
    PREC_BIT_XOR,   ///< Bitwise Exclusive OR
    PREC_BIT_AND,   ///< Bitwise AND
    PREC_EQUAL,     ///< Equality
    PREC_REL,       ///< Relational
    PREC_BIT_SHIFT, ///< Bitwise Shift
    PREC_ADD,       ///< Additive
    PREC_MULT,      ///< Multiplicative
    PREC_PTR_MEM,   ///< C++ Pointer to Member
    PREC_UNARY,     ///< Unary
    PREC_PRIM,      ///< Primary
    PREC_SCOPE      ///< Primary scope resolution
};


/**
 * Concrete class for the "C" high level language
 * This class provides methods which are specific for the C language binding.
 * I guess this will be the most popular output language unless we do C++.
 */
class CCodeGenerator : public ICodeGenerator
{
public:
    CCodeGenerator() = default;
    virtual ~CCodeGenerator() override = default;

public:
    /// \copydoc ICodeGenerator::generateCode
    virtual void generateCode(const Prog *prog, QTextStream& os) override;

    /// \copydoc ICodeGenerator::generateCode
    virtual void generateCode(const Prog *prog, Module *module = nullptr, UserProc *proc = nullptr, bool intermixRTL = false) override;

public:
    /// \copydoc ICodeGenerator::addAssignmentStatement
    virtual void addAssignmentStatement(Assign *assign) override;

    /// \copydoc ICodeGenerator::addCallStatement
    virtual void addCallStatement(Function *func, const QString& name,
                                  const StatementList& args, const StatementList& results) override;

    /// \copydoc ICodeGenerator::addCallStatement
    virtual void addIndCallStatement(const SharedExp& exp, const StatementList& args,
                                     const StatementList& results) override;

    /// \copydoc ICodeGenerator::addReturnStatement
    virtual void addReturnStatement(StatementList *rets) override;

    /**
     * \copydoc ICodeGenerator::removeUnusedLabels
     */
    virtual void removeUnusedLabels(int) override;

private:
    /// Add a prototype (for forward declaration)
    void addPrototype(UserProc *proc);

    /// Generate code for a single procedure.
    void generateCode(UserProc *proc);

    /// Generate global variables from data sections.
    void generateDataSectionCode(IBinaryImage *image, QString sectionName, Address sectionStart, uint32_t sectionSize);

    /**
     * Print the declaration of a function.
     * \param proc Function to print
     * \param isDef True to print trailing opening bracket '{', false to print ';'
     */
    void addFunctionSignature(UserProc *proc, bool isDef);

    /*
     * Functions to add new code
     */

    // pretested loops (cond is optional because it is in the bb [somewhere])
    /// Adds: while (\p cond) {
    void addPretestedLoopHeader(const SharedExp& cond);

    /// Adds: }
    void addPretestedLoopEnd();

    // endless loops
    /// Adds: for(;;) {
    void addEndlessLoopHeader();

    /// Adds: }
    void addEndlessLoopEnd();

    // posttested loops
    /// Adds: do {
    void addPostTestedLoopHeader();

    /// Adds: } while (\a cond);
    void addPostTestedLoopEnd(const SharedExp& cond);

    // case conditionals "nways"
    /// Adds: switch(\a cond) {
    void addCaseCondHeader(const SharedExp& cond);

    /// Adds: case \a opt :
    void addCaseCondOption(Exp& opt);

    /// Adds: break;
    void addCaseCondOptionEnd();

    /// Adds: default:
    void addCaseCondElse();

    /// Adds: }
    void addCaseCondEnd();

    // if conditions
    /// Adds: if(\a cond) {
    void addIfCondHeader(const SharedExp& cond);

    /// Adds: }
    void addIfCondEnd();

    // if else conditions
    /// Adds: if(\a cond) {
    void addIfElseCondHeader(const SharedExp& cond);

    /// Adds: } else {
    void addIfElseCondOption();

    /// Adds: }
    void addIfElseCondEnd();

    // goto, break, continue, etc
    /// Adds: goto L \em ord
    void addGoto(int ord);

    /// Adds: continue;
    void addContinue();

    /// Adds: break;
    void addBreak();

    // labels
    /// Adds: L \a ord :
    void addLabel(int ord);

    /// Search for the label L \a ord and remove it from the generated code.
    void removeLabel(int ord);

    // proc related
    /**
     * Print the start of a function, and also as a comment its address.
     */
    void addProcStart(UserProc *proc);

    /// Adds: }
    void addProcEnd();

    /**
     * Declare a local variable.
     * \param name given to the new local
     * \param type of this local variable
     * \param last true if an empty line should be added.
     */
    void addLocal(const QString& name, SharedType type, bool last = false);

    /**
     * Add the declaration for a global.
     * \param name given name for the global
     * \param type The type of the global
     * \param init The initial value of the global.
     */
    void addGlobal(const QString& name, SharedType type, const SharedExp& init = nullptr);

    /// Adds one line of comment to the code.
    void addLineComment(const QString& cmt);

private:
    /**
     * Append code for the given expression \a exp to stream \a str.
     *
     * \param str           The stream to output to.
     * \param exp           The expresson to output.
     * \param curPrec       The current operator precedence. Add parens around this expression if necessary.
     * \param allowUnsigned If true, cast operands to unsigned if necessary.
     *
     * \todo This function is 800+ lines, and should possibly be split up.
     */
    void appendExp(QTextStream& str, const Exp& exp, PREC curPrec, bool allowUnsigned = false);

    /// Print the type represented by \a typ to \a str.
    void appendType(QTextStream& str, SharedType typ);

    /**
     * Print the identified type to \a str.
     */
    void appendTypeIdent(QTextStream& str, SharedType typ, QString ident);

    /// Adds: (
    void openParen(QTextStream& str, PREC outer, PREC inner);

    /// Adds: )
    void closeParen(QTextStream& str, PREC outer, PREC inner);


    void generateCode(BasicBlock *bb, BasicBlock *latch, std::list<BasicBlock *>& followSet, std::list<BasicBlock *>& gotoSet, UserProc *proc);
    void generateCode_Loop(BasicBlock *bb, std::list<BasicBlock *>& gotoSet, UserProc *proc, BasicBlock *latch, std::list<BasicBlock *>& followSet);

    /// Emits a goto statement (at the correct indentation level) with the destination label for dest. Also places the label
    /// just before the destination code if it isn't already there.    If the goto is to the return block, it would be nice
    /// to
    /// emit a 'return' instead (but would have to duplicate the other code in that return BB).    Also, 'continue' and
    /// 'break'
    /// statements are used instead if possible
    void emitGotoAndLabel(BasicBlock *bb, BasicBlock *dest);

    /// Generates code for each non-CTI (except procedure calls) statement within the block.
    void writeBB(const BasicBlock *bb);

    /// \returns true if all predecessors of this BB have had their code generated.
    bool isAllParentsGenerated(const BasicBlock *bb) const;
    bool isGenerated(const BasicBlock *bb) const;

private:
    /// Dump all generated code to \p os.
    void print(QTextStream& os);

    /// Output 4 * \p indLevel spaces to \p str
    void indent(QTextStream& str, int indLevel);

    /// Private helper functions, to reduce redundant code, and
    /// have a single place to put a breakpoint on.
    void appendLine(const QString& s);

private:
    int m_indent = 0;                       ///< Current indentation depth
    std::map<QString, SharedType> m_locals; ///< All locals in a Proc
    std::set<int> m_usedLabels;             ///< All used goto labels.
    QStringList m_lines;                    ///< The generated code.
    UserProc *m_proc = nullptr;
    std::unordered_set<const BasicBlock *> m_generatedBBs;
};
