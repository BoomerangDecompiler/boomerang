#pragma once

#include <QTextStream>

class BasicBlock;

/**
 * Base class for all nodes in the Abstract Syntax Tree.
 */
class SyntaxNode
{
public:
    SyntaxNode();
    virtual ~SyntaxNode();

    virtual bool isBlock() const { return false; }
    virtual bool isGoto() const;
    virtual bool isBranch() const;

    virtual void ignoreGoto() {}

    virtual int getNumber() const { return m_nodeID; }

    BasicBlock *getBB() const { return m_bb; }
    void setBB(BasicBlock *bb) { m_bb = bb; }

    virtual size_t getNumOutEdges() const = 0;
    virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) = 0;
    virtual bool endsWithGoto() const = 0;

    virtual bool startsWith(SyntaxNode *node) const { return this == node; }

    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr) = 0;

    int getScore();

    void addToScore(int n) { m_score = getScore() + n; }
    void setDepth(int n) { m_depth = n; }
    int getDepth() const { return m_depth; }

    virtual SyntaxNode *clone() = 0;
    virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) = 0;

    SyntaxNode *getCorrespond() const { return m_correspond; }

    virtual SyntaxNode *findNodeFor(BasicBlock *bb)          = 0;
    virtual void printAST(SyntaxNode *root, QTextStream& os) = 0;
    virtual int evaluate(SyntaxNode *root) = 0;

    virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
    { Q_UNUSED(root); Q_UNUSED(successors); }

protected:
    BasicBlock *m_bb;
    int m_nodeID;
    int m_score;
    SyntaxNode *m_correspond; ///< corresponding node in previous state
    bool m_isGoto;
    int m_depth;
};

// for debugging
void PRINT_BEFORE_AFTER(SyntaxNode *root, SyntaxNode *n);
