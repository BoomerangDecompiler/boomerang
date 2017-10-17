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


#include "boomerang/codegen/syntax/SyntaxNode.h"
#include "boomerang/db/exp/ExpHelp.h"


class IfThenSyntaxNode : public SyntaxNode
{
public:
    IfThenSyntaxNode();
    virtual ~IfThenSyntaxNode();

    virtual bool isGoto() const override { return false; }
    virtual bool isBranch() const override { return false; }

    virtual size_t getNumOutEdges() const override { return 1; }
    virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t) override;
    virtual bool endsWithGoto() const override { return false; }

    virtual SyntaxNode *clone() override;
    virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr)  override
    {
        if (this == base) {
            return cur;
        }

        return m_then->getEnclosingLoop(base, cur);
    }

    void setCond(SharedExp e) { m_cond = e; }
    SharedExp getCond() const { return m_cond; }
    void setThen(SyntaxNode *n) { m_then = n; }

    virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
    virtual void printAST(SyntaxNode *root, QTextStream& os) override;
    virtual int evaluate(SyntaxNode *root) override;
    virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;

protected:
    SyntaxNode *m_then;
    SharedExp m_cond;
};


class IfThenElseSyntaxNode : public SyntaxNode
{
public:
    IfThenElseSyntaxNode();
    virtual ~IfThenElseSyntaxNode();

    virtual bool isGoto() const override { return false; }
    virtual bool isBranch() const override { return false; }

    virtual size_t getNumOutEdges() const override { return 1; }

    virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t)  override;

    virtual bool endsWithGoto() const override { return false; }

    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr)  override;

    virtual SyntaxNode *clone() override;
    virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

    void setCond(SharedExp e) { m_cond = e; }
    void setThen(SyntaxNode *n) { m_then = n; }
    void setElse(SyntaxNode *n) { m_else = n; }

    virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
    virtual void printAST(SyntaxNode *root, QTextStream& os) override;
    virtual int evaluate(SyntaxNode *root) override;
    virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;

protected:
    SyntaxNode *m_then;
    SyntaxNode *m_else;
    SharedExp m_cond;
};
