#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "IfThenSyntaxNode.h"


#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/exp/Exp.h"
#include "boomerang/util/Log.h"


IfThenSyntaxNode::IfThenSyntaxNode()
    : m_then(nullptr)
    , m_cond(nullptr)
{
}


IfThenSyntaxNode::~IfThenSyntaxNode()
{
    delete m_then;
}


SyntaxNode *IfThenSyntaxNode::getOutEdge(SyntaxNode *root, size_t)
{
    SyntaxNode *n1 = root->findNodeFor(m_bb->getOutEdge(0));

    assert(n1 != m_then);
    return n1;
}


int IfThenSyntaxNode::evaluate(SyntaxNode *root)
{
    int n = 1;

    n += m_then->evaluate(root);
    return n;
}


void IfThenSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
    m_then->addSuccessors(root, successors);
}


SyntaxNode *IfThenSyntaxNode::clone()
{
    IfThenSyntaxNode *b = new IfThenSyntaxNode();

    b->m_correspond = this;
    b->m_bb         = m_bb;
    b->m_cond       = m_cond->clone();
    b->m_then       = m_then->clone();
    return b;
}


SyntaxNode *IfThenSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
    assert(m_correspond != from);

    if (m_then->getCorrespond() == from) {
        assert(to);
        m_then = to;
    }
    else {
        m_then = m_then->replace(from, to);
    }

    return this;
}


SyntaxNode *IfThenSyntaxNode::findNodeFor(BasicBlock *bb)
{
    if (m_bb == bb) {
        return this;
    }

    return m_then->findNodeFor(bb);
}


void IfThenSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << "[label=\"if " << m_cond << " \"];" << '\n';
    m_then->printAST(root, os);
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << m_then->getNumber() << " [label=then];" << '\n';
    SyntaxNode *follows = root->findNodeFor(m_bb->getOutEdge(0));
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << follows->getNumber() << " [style=dotted];" << '\n';
}


IfThenElseSyntaxNode::IfThenElseSyntaxNode()
    : m_then(nullptr)
    , m_else(nullptr)
    , m_cond(nullptr)
{
}


IfThenElseSyntaxNode::~IfThenElseSyntaxNode()
{
    if (m_then) {
        delete m_then;
    }

    if (m_else) {
        delete m_else;
    }
}


int IfThenElseSyntaxNode::evaluate(SyntaxNode *root)
{
    int n = 1;

    n += m_then->evaluate(root);
    n += m_else->evaluate(root);
    return n;
}


void IfThenElseSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
    // at the moment we can always ignore gotos at the end of
    // then and else, because we assume they have the same
    // follow
    if ((m_then->getNumOutEdges() == 1) && m_then->endsWithGoto()) {
        LOG_VERBOSE("Successor: ignoring goto at end of then of if then else");
        SyntaxNode *n = root->clone();
        n->setDepth(root->getDepth() + 1);
        SyntaxNode *nThen = m_then->clone();
        nThen->ignoreGoto();
        n = n->replace(m_then, nThen);
        successors.push_back(n);
    }

    if ((m_else->getNumOutEdges() == 1) && m_else->endsWithGoto()) {
        LOG_VERBOSE("Successor: ignoring goto at end of else of if then else");
        SyntaxNode *n = root->clone();
        n->setDepth(root->getDepth() + 1);
        SyntaxNode *nElse = m_else->clone();
        nElse->ignoreGoto();
        n = n->replace(m_else, nElse);
        successors.push_back(n);
    }

    m_then->addSuccessors(root, successors);
    m_else->addSuccessors(root, successors);
}


SyntaxNode *IfThenElseSyntaxNode::clone()
{
    IfThenElseSyntaxNode *b = new IfThenElseSyntaxNode();

    b->m_correspond = this;
    b->m_bb         = m_bb;
    b->m_cond       = m_cond->clone();
    b->m_then       = m_then->clone();
    b->m_else       = m_else->clone();
    return b;
}


SyntaxNode *IfThenElseSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
    assert(m_correspond != from);

    if (m_then->getCorrespond() == from) {
        assert(to);
        m_then = to;
    }
    else {
        m_then = m_then->replace(from, to);
    }

    if (m_else->getCorrespond() == from) {
        assert(to);
        m_else = to;
    }
    else {
        m_else = m_else->replace(from, to);
    }

    return this;
}


SyntaxNode *IfThenElseSyntaxNode::findNodeFor(BasicBlock *bb)
{
    if (m_bb == bb) {
        return this;
    }

    SyntaxNode *n = m_then->findNodeFor(bb);

    if (n == nullptr) {
        n = m_else->findNodeFor(bb);
    }

    return n;
}


void IfThenElseSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << "[label=\"if " << m_cond << " \"];" << '\n';
    m_then->printAST(root, os);
    m_else->printAST(root, os);
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << m_then->getNumber() << " [label=then];" << '\n';
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << m_else->getNumber() << " [label=else];" << '\n';
}


SyntaxNode *IfThenElseSyntaxNode::getOutEdge(SyntaxNode *root, size_t)
{
    SyntaxNode *o = m_then->getOutEdge(root, 0);

    assert(o == m_else->getOutEdge(root, 0));
    return o;
}


SyntaxNode *IfThenElseSyntaxNode::getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur)
{
    if (this == base) {
        return cur;
    }

    SyntaxNode *n = m_then->getEnclosingLoop(base, cur);
    return n ? n : m_else->getEnclosingLoop(base, cur);
}
