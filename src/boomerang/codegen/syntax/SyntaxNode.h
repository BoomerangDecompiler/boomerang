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


#include <QTextStream>

class BasicBlock;


/**
 * Base class for all nodes in the Abstract Syntax Tree.
 */
class SyntaxNode
{
public:
    SyntaxNode();
    SyntaxNode(const SyntaxNode& other) = delete;
    SyntaxNode(SyntaxNode&& other) = default;

    virtual ~SyntaxNode();

    SyntaxNode& operator=(const SyntaxNode& other) = delete;
    SyntaxNode& operator=(SyntaxNode&& other) = default;

public:
    /// \returns true if this is a fully generated block syntax node
    virtual bool isBlock() const { return false; }

    /// \returns true if this is a branch
    virtual bool isBranch() const;

    /// \returns true if this syntax node is a goto
    virtual bool isGoto() const;

    /// \returns true if this syntax node ends with a goto
    virtual bool endsWithGoto() const = 0;

    /// Ignore the goto statement in this node, if applicable.
    /// May happen e.g. when a goto would jump directly to the next expression in the code.
    virtual void ignoreGoto() {}

    /// \returns the unique identifier for this syntax node.
    virtual int getNumber() const { return m_nodeID; }

    /// \returns the BasicBlock associated with this node.
    BasicBlock *getBB() const { return m_bb; }

    /// Updates the BasicBlock associated with this node.
    void setBB(BasicBlock *bb) { m_bb = bb; }

    /// \returns the number of out edges immediately following this node.
    /// For infinite loops, this must be zero.
    virtual size_t getNumOutEdges() const = 0;

    /// \returns the target node of the \p n -th out edge.
    virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) = 0;

    /// \returns true if the first elements of this node are the same as the elements of \p node
    virtual bool startsWith(SyntaxNode *node) const { return this == node; }

    ///
    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr) = 0;

    /// Get or compute the importance score of this node.
    int getScore();

    /// Add \p n to the score of this node.
    void addToScore(int n) { m_score = getScore() + n; }

    /// Update the base indent of this node.
    void setDepth(int n) { m_depth = n; }

    /// \returns the base indent of this node.
    int getDepth() const { return m_depth; }

    /// Make a deep copy of this
    virtual SyntaxNode *clone() = 0;

    /// Replace all instances of \p from in all children of this by \p to
    virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) = 0;

    /// \returns the corresponding node in a previous state.
    /// For example, if p is a clone of this, then p->getCorrespond() == this.
    SyntaxNode *getCorrespond() const { return m_correspond; }

    /// \returns the syntax node containing \p bb
    virtual SyntaxNode *findNodeFor(BasicBlock *bb) = 0;

    /// Print the Syntax Tree to \p os
    virtual void printAST(SyntaxNode *root, QTextStream& os) = 0;

    /// Evaluate the score of this node
    /// \sa getScore()
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
