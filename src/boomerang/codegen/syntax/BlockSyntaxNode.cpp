#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BlockSyntaxNode.h"


#include "boomerang/db/BasicBlock.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/exp/Unary.h"

#include "boomerang/codegen/syntax/IfThenSyntaxNode.h"
#include "boomerang/codegen/syntax/LoopSyntaxNode.h"


BlockSyntaxNode::BlockSyntaxNode()
{
}


BlockSyntaxNode::~BlockSyntaxNode()
{
    for (auto& elem : statements) {
        delete elem;
    }
}


size_t BlockSyntaxNode::getNumOutEdges() const
{
    if (m_bb) {
        return m_bb->getNumOutEdges();
    }

    if (statements.size() == 0) {
        return 0;
    }

    return statements[statements.size() - 1]->getNumOutEdges();
}


SyntaxNode *BlockSyntaxNode::getOutEdge(SyntaxNode *root, size_t n)
{
    if (m_bb) {
        return root->findNodeFor(m_bb->getOutEdge(n));
    }

    if (statements.size() == 0) {
        return nullptr;
    }

    return statements[statements.size() - 1]->getOutEdge(root, n);
}


SyntaxNode *BlockSyntaxNode::findNodeFor(BasicBlock *bb)
{
    if (m_bb == bb) {
        return this;
    }

    SyntaxNode *n = nullptr;

    for (auto& elem : statements) {
        n = elem->findNodeFor(bb);

        if (n != nullptr) {
            break;
        }
    }

    if (n && (n == statements[0])) {
        return this;
    }

    return n;
}


void BlockSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";

    os << "[label=\"";

    if (m_bb) {
        switch (m_bb->getType())
        {
        case BBType::Oneway:
            os << "Oneway";

            if (!m_isGoto) {
                os << " (ignored)";
            }

            break;

        case BBType::Twoway:
            os << "Twoway";
            break;

        case BBType::Nway:
            os << "Nway";
            break;

        case BBType::Call:
            os << "Call";
            break;

        case BBType::Ret:
            os << "Ret";
            break;

        case BBType::Fall:
            os << "Fall";
            break;

        case BBType::CompJump:
            os << "Computed jump";
            break;

        case BBType::CompCall:
            os << "Computed call";
            break;

        case BBType::Invalid:
            os << "Invalid";
            break;
        }

        os << " " << m_bb->getLowAddr();
    }
    else {
        os << "block";
    }

    os << "\"];" << '\n';

    if (m_bb) {
        for (size_t i = 0; i < m_bb->getNumOutEdges(); i++) {
            BasicBlock *out = m_bb->getOutEdge(i);
            os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";

            SyntaxNode *to = root->findNodeFor(out);
            assert(to);
            os << " -> " << to->getNumber() << " [style=dotted";

            if (m_bb->getNumOutEdges() > 1) {
                os << ",label=" << i;
            }

            os << "];" << '\n';
        }
    }
    else {
        for (auto& elem : statements) {
            elem->printAST(root, os);
        }

        for (unsigned i = 0; i < statements.size(); i++) {
            os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
            os << " -> " << statements[i]->getNumber() << " [label=\"" << i << "\"];" << '\n';
        }
    }
}


#define DEBUG_EVAL    0

int BlockSyntaxNode::evaluate(SyntaxNode *root)
{
#if DEBUG_EVAL
    if (this == root) {
        LOG_MSG("begin eval =============");
    }
#endif

    if (m_bb) {
        return 1;
    }

    int n = 1;

    if (statements.size() == 1) {
        SyntaxNode *out = statements[0]->getOutEdge(root, 0);

        if ((out->getBB() != nullptr) && (out->getBB()->getNumInEdges() > 1)) {
#if DEBUG_EVAL
            LOG_MSG("Add 15");
#endif
            n += 15;
        }
        else {
#if DEBUG_EVAL
            LOG_MSG("Add 30");
#endif
            n += 30;
        }
    }

    for (unsigned i = 0; i < statements.size(); i++) {
        n += statements[i]->evaluate(root);

        if (statements[i]->isGoto()) {
            if (i != statements.size() - 1) {
#if DEBUG_EVAL
                LOG_MSG("Add 100");
#endif
                n += 100;
            }
            else {
#if DEBUG_EVAL
                LOG_MSG("Add 50");
#endif
                n += 50;
            }
        }
        else if (statements[i]->isBranch()) {
            SyntaxNode *loop = root->getEnclosingLoop(this);
            LOG_VERBOSE("Branch %1 not in loop", statements[i]->getNumber());

            if (loop) {
                LOG_VERBOSE("Branch %1 in loop %2", statements[i]->getNumber(), loop->getNumber());
                // this is a bit C specific
                SyntaxNode *out = loop->getOutEdge(root, 0);

                if (out && (statements[i]->getOutEdge(root, 0) == out)) {
                    LOG_VERBOSE("found break");
                    n += 10;
                }

                if (statements[i]->getOutEdge(root, 0) == loop) {
                    LOG_VERBOSE("found continue");
                    n += 10;
                }
            }
            else {
#if DEBUG_EVAL
                LOG_MSG("add 50");
#endif
                n += 50;
            }
        }
        else if ((i < statements.size() - 1) && (statements[i]->getOutEdge(root, 0) != statements[i + 1])) {
#if DEBUG_EVAL
            LOG_MSG("Add 25");
            LOG_MSG("%1 -> %2 not %3",
                    statements[i]->getNumber(),
                    statements[i]->getOutEdge(root, 0)->getNumber()
                    statements[i + 1]->getNumber());
#endif
            n += 25;
        }
    }

#if DEBUG_EVAL
    if (this == root) {
        LOG_MSG("End eval = %1 =============", n);
    }
#endif
    return n;
}


void BlockSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
    for (unsigned i = 0; i < statements.size(); i++) {
        if (statements[i]->isBlock()) {
            // BlockSyntaxNode *b = (BlockSyntaxNode*)statements[i];
            // can move previous statements into this block
            if (i > 0) {
                LOG_VERBOSE("Successor: move previous statement into block");
                SyntaxNode *n = root->clone();
                n->setDepth(root->getDepth() + 1);
                BlockSyntaxNode *b1 = (BlockSyntaxNode *)this->clone();
                BlockSyntaxNode *nb = (BlockSyntaxNode *)b1->getStatement(i);
                b1 = (BlockSyntaxNode *)b1->replace(statements[i - 1], nullptr);
                nb->prependStatement(statements[i - 1]->clone());
                n = n->replace(this, b1);
                successors.push_back(n);
                // PRINT_BEFORE_AFTER
            }
        }
        else {
            if (statements.size() != 1) {
                // can replace statement with a block containing that statement
                LOG_VERBOSE("Successor: replace statement with a block containing the statement");

                BlockSyntaxNode *b = new BlockSyntaxNode();
                b->addStatement(statements[i]->clone());
                SyntaxNode *n = root->clone();
                n->setDepth(root->getDepth() + 1);
                n = n->replace(statements[i], b);
                successors.push_back(n);
                // PRINT_BEFORE_AFTER
            }
        }

        // "jump over" style of if-then
        if ((i < statements.size() - 2) && statements[i]->isBranch()) {
            SyntaxNode *b = statements[i];

            if ((b->getOutEdge(root, 0) == statements[i + 2]) &&
                ((statements[i + 1]->getOutEdge(root, 0) == statements[i + 2]) || statements[i + 1]->endsWithGoto())) {
                LOG_VERBOSE("Successor: jump over style if then");

                BlockSyntaxNode *b1 = (BlockSyntaxNode *)this->clone();
                b1 = (BlockSyntaxNode *)b1->replace(statements[i + 1], nullptr);
                IfThenSyntaxNode *nif = new IfThenSyntaxNode();
                SharedExp        cond = b->getBB()->getCond();
                cond = Unary::get(opLNot, cond->clone());
                cond = cond->simplify();
                nif->setCond(cond);
                nif->setThen(statements[i + 1]->clone());
                nif->setBB(b->getBB());
                b1->setStatement(i, nif);
                SyntaxNode *n = root->clone();
                n->setDepth(root->getDepth() + 1);
                n = n->replace(this, b1);
                successors.push_back(n);
                // PRINT_BEFORE_AFTER
            }
        }

        // if then else
        if ((i < statements.size() - 2) && statements[i]->isBranch()) {
            SyntaxNode *tThen = statements[i]->getOutEdge(root, 0);
            SyntaxNode *tElse = statements[i]->getOutEdge(root, 1);

            assert(tThen && tElse);

            if ((((tThen == statements[i + 2]) && (tElse == statements[i + 1])) ||
                 ((tThen == statements[i + 1]) && (tElse == statements[i + 2]))) &&
                (tThen->getNumOutEdges() == 1) && (tElse->getNumOutEdges() == 1)) {
                SyntaxNode *else_out = tElse->getOutEdge(root, 0);
                SyntaxNode *then_out = tThen->getOutEdge(root, 0);

                if (else_out == then_out) {
                    LOG_VERBOSE("Successor: if then else");

                    SyntaxNode *n = root->clone();
                    n->setDepth(root->getDepth() + 1);
                    n = n->replace(tThen, nullptr);
                    n = n->replace(tElse, nullptr);
                    IfThenElseSyntaxNode *nif = new IfThenElseSyntaxNode();
                    nif->setCond(statements[i]->getBB()->getCond()->clone());
                    nif->setBB(statements[i]->getBB());
                    nif->setThen(tThen->clone());
                    nif->setElse(tElse->clone());
                    n = n->replace(statements[i], nif);
                    successors.push_back(n);
                    // PRINT_BEFORE_AFTER
                }
            }
        }

        // pretested loop
        if ((i < statements.size() - 2) && statements[i]->isBranch()) {
            SyntaxNode *tBody   = statements[i]->getOutEdge(root, 0);
            SyntaxNode *tFollow = statements[i]->getOutEdge(root, 1);

            assert(tBody && tFollow);

            if ((tBody == statements[i + 1]) && (tFollow == statements[i + 2]) && (tBody->getNumOutEdges() == 1) &&
                (tBody->getOutEdge(root, 0) == statements[i])) {
                LOG_VERBOSE("successor: pretested loop");
                SyntaxNode *n = root->clone();
                n->setDepth(root->getDepth() + 1);
                n = n->replace(tBody, nullptr);
                LoopSyntaxNode *nloop = new LoopSyntaxNode(tBody->clone(),
                                                           statements[i]->getBB()->getCond()->clone(),
                                                           false);

                nloop->setBB(statements[i]->getBB());
                n = n->replace(statements[i], nloop);
                successors.push_back(n);
                // PRINT_BEFORE_AFTER
            }
        }

        // posttested loop
        if ((i > 0) && (i < statements.size() - 1) && statements[i]->isBranch()) {
            SyntaxNode *tBody   = statements[i]->getOutEdge(root, 0);
            SyntaxNode *tFollow = statements[i]->getOutEdge(root, 1);

            assert(tBody && tFollow);

            if ((tBody == statements[i - 1]) && (tFollow == statements[i + 1]) && (tBody->getNumOutEdges() == 1) &&
                (tBody->getOutEdge(root, 0) == statements[i])) {
                LOG_VERBOSE("Successor: posttested loop");
                SyntaxNode *n = root->clone();
                n->setDepth(root->getDepth() + 1);
                n = n->replace(tBody, nullptr);
                LoopSyntaxNode *nloop = new LoopSyntaxNode(tBody->clone(),
                                                           statements[i]->getBB()->getCond()->clone(),
                                                           true);
                nloop->setBB(statements[i]->getBB());
                n = n->replace(statements[i], nloop);
                successors.push_back(n);
                // PRINT_BEFORE_AFTER
            }
        }

        // infinite loop
        if ((statements[i]->getNumOutEdges() == 1) && (statements[i]->getOutEdge(root, 0) == statements[i])) {
            LOG_VERBOSE("Successor: infinite loop");
            SyntaxNode *n = root->clone();
            n->setDepth(root->getDepth() + 1);
            LoopSyntaxNode *nloop = new LoopSyntaxNode(statements[i]->clone());
            n = n->replace(statements[i], nloop);
            successors.push_back(n);
            PRINT_BEFORE_AFTER(root, n);
        }

        statements[i]->addSuccessors(root, successors);
    }
}


SyntaxNode *BlockSyntaxNode::clone()
{
    BlockSyntaxNode *b = new BlockSyntaxNode();

    b->m_correspond = this;

    if (m_bb) {
        b->m_bb = m_bb;
    }
    else {
        for (auto& elem : statements) {
            b->addStatement(elem->clone());
        }
    }

    return b;
}


SyntaxNode *BlockSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
    if (m_correspond == from) {
        return to;
    }

    if (m_bb == nullptr) {
        std::vector<SyntaxNode *> news;

        for (auto& elem : statements) {
            SyntaxNode *n = elem;

            if (elem->getCorrespond() == from) {
                n = to;
            }
            else {
                n = elem->replace(from, to);
            }

            if (n) {
                news.push_back(n);
            }
        }

        statements.resize(news.size());

        for (unsigned i = 0; i < news.size(); i++) {
            statements[i] = news[i];
        }
    }

    return this;
}


bool BlockSyntaxNode::isBlock() const
{
    return m_bb == nullptr;
}


void BlockSyntaxNode::ignoreGoto()
{
    if (m_bb) {
        m_isGoto = false;
    }
    else if (statements.size() > 0) {
        statements[statements.size() - 1]->ignoreGoto();
    }
}


SyntaxNode *BlockSyntaxNode::getStatement(size_t n)
{
    assert(m_bb == nullptr);
    assert(n < statements.size());
    return statements[n];
}


void BlockSyntaxNode::prependStatement(SyntaxNode *n)
{
    assert(m_bb == nullptr);
    statements.resize(statements.size() + 1);

    for (size_t i = statements.size() - 1; i > 0; i--) {
        statements[i] = statements[i - 1];
    }

    statements[0] = n;
}


void BlockSyntaxNode::addStatement(SyntaxNode *n)
{
    assert(m_bb == nullptr);
    statements.push_back(n);
}


void BlockSyntaxNode::setStatement(size_t i, SyntaxNode *n)
{
    assert(m_bb == nullptr);
    statements[i] = n;
}


bool BlockSyntaxNode::endsWithGoto() const
{
    if (m_bb) {
        return isGoto();
    }

    bool last = false;

    if (statements.size() > 0) {
        last = statements[statements.size() - 1]->endsWithGoto();
    }

    return last;
}


bool BlockSyntaxNode::startsWith(SyntaxNode *node) const
{
    return this == node || (statements.size() > 0 && statements[0]->startsWith(node));
}


SyntaxNode *BlockSyntaxNode::getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur)
{
    if (this == base) {
        return cur;
    }

    for (unsigned i = 0; i < statements.size(); i++) {
        SyntaxNode *n = statements[i]->getEnclosingLoop(base, cur);

        if (n) {
            return n;
        }
    }

    return nullptr;
}


size_t BlockSyntaxNode::getNumStatements() const
{
    return m_bb ? 0 : statements.size();
}
