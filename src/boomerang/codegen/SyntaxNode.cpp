#include "SyntaxNode.h"

#include "boomerang/util/Log.h"
#include "boomerang/core/Boomerang.h"

#include "boomerang/db/Prog.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/exp/Unary.h"

#include <iomanip> // For setfill etc
#include <cstring>
#include <cstdlib>

static int nodecount = 1000;

void PRINT_BEFORE_AFTER(SyntaxNode *root, SyntaxNode *n)
{
    QFile tgt("before.dot");

    if (!tgt.open(QFile::WriteOnly)) {
        return;
    }

    QTextStream of(&tgt);
    of << "digraph before {" << '\n';
    root->printAST(root, of);
    of << "}" << '\n';
    QFile tgt2("after.dot");

    if (!tgt2.open(QFile::WriteOnly)) {
        return;
    }

    QTextStream of1(&tgt2);
    of1 << "digraph after {" << '\n';
    n->printAST(n, of1);
    of1 << "}" << '\n';
    exit(0);
}


SyntaxNode::SyntaxNode()
    : m_bb(nullptr)
    , m_score(-1)
    , m_correspond(nullptr)
    , m_isGoto(true)
{
    m_nodeID = nodecount++;
}


SyntaxNode::~SyntaxNode()
{
}


int SyntaxNode::getScore()
{
    if (m_score == -1) {
        m_score = evaluate(this);
    }

    return m_score;
}


bool SyntaxNode::isGoto() const
{
    return m_bb && m_bb->getType() == BBType::Oneway && m_isGoto;
}


bool SyntaxNode::isBranch() const
{
    return m_bb && m_bb->getType() == BBType::Twoway;
}


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
                PretestedLoopSyntaxNode *nloop = new PretestedLoopSyntaxNode();
                nloop->setCond(statements[i]->getBB()->getCond()->clone());
                nloop->setBB(statements[i]->getBB());
                nloop->setBody(tBody->clone());
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
                PostTestedLoopSyntaxNode *nloop = new PostTestedLoopSyntaxNode();
                nloop->setCond(statements[i]->getBB()->getCond()->clone());
                nloop->setBB(statements[i]->getBB());
                nloop->setBody(tBody->clone());
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
            InfiniteLoopSyntaxNode *nloop = new InfiniteLoopSyntaxNode();
            nloop->setBody(statements[i]->clone());
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
    b->m_bb        = m_bb;
    b->m_cond         = m_cond->clone();
    b->m_then        = m_then->clone();
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
        LOG_VERBOSE("Successor: ignoring goto at end of else of if then else");;
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
    b->m_bb        = m_bb;
    b->m_cond         = m_cond->clone();
    b->m_then        = m_then->clone();
    b->m_else        = m_else->clone();
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


PretestedLoopSyntaxNode::PretestedLoopSyntaxNode()
    : m_body(nullptr)
    , m_cond(nullptr)
{
}


PretestedLoopSyntaxNode::~PretestedLoopSyntaxNode()
{
    if (m_body) {
        delete m_body;
    }
}


SyntaxNode *PretestedLoopSyntaxNode::getOutEdge(SyntaxNode *root, size_t)
{
    return root->findNodeFor(m_bb->getOutEdge(1));
}


int PretestedLoopSyntaxNode::evaluate(SyntaxNode *root)
{
    int n = 1;

    n += m_body->evaluate(root);
    return n;
}


void PretestedLoopSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
    // we can always ignore gotos at the end of the body.
    if ((m_body->getNumOutEdges() == 1) && m_body->endsWithGoto()) {
        LOG_VERBOSE("successor: ignoring goto at end of body of pretested loop");
        assert(m_body->getOutEdge(root, 0)->startsWith(this));

        SyntaxNode *n = root->clone();
        n->setDepth(root->getDepth() + 1);
        SyntaxNode *nBody = m_body->clone();
        nBody->ignoreGoto();
        n = n->replace(m_body, nBody);
        successors.push_back(n);
    }

    m_body->addSuccessors(root, successors);
}


SyntaxNode *PretestedLoopSyntaxNode::clone()
{
    PretestedLoopSyntaxNode *b = new PretestedLoopSyntaxNode();

    b->m_correspond = this;
    b->m_bb        = m_bb;
    b->m_cond         = m_cond->clone();
    b->m_body        = m_body->clone();
    return b;
}


SyntaxNode *PretestedLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
    assert(m_correspond != from);

    if (m_body->getCorrespond() == from) {
        assert(to);
        m_body = to;
    }
    else {
        m_body = m_body->replace(from, to);
    }

    return this;
}


SyntaxNode *PretestedLoopSyntaxNode::findNodeFor(BasicBlock *bb)
{
    if (m_bb == bb) {
        return this;
    }

    return m_body->findNodeFor(bb);
}


void PretestedLoopSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << "[label=\"loop pretested ";
    os << m_cond << " \"];" << '\n';
    m_body->printAST(root, os);
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << m_body->getNumber() << ";" << '\n';
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << getOutEdge(root, 0)->getNumber() << " [style=dotted];" << '\n';
}


PostTestedLoopSyntaxNode::PostTestedLoopSyntaxNode()
    : m_body(nullptr)
    , m_cond(nullptr)
{
}


PostTestedLoopSyntaxNode::~PostTestedLoopSyntaxNode()
{
    if (m_body) {
        delete m_body;
    }
}


SyntaxNode *PostTestedLoopSyntaxNode::getOutEdge(SyntaxNode *root, size_t)
{
    return root->findNodeFor(m_bb->getOutEdge(1));
}


int PostTestedLoopSyntaxNode::evaluate(SyntaxNode *root)
{
    int n = 1;

    n += m_body->evaluate(root);
    return n;
}


void PostTestedLoopSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
    // we can always ignore gotos at the end of the body.
    if ((m_body->getNumOutEdges() == 1) && m_body->endsWithGoto()) {
        LOG_VERBOSE("successor: ignoring goto at end of body of posttested loop");
        assert(m_body->getOutEdge(root, 0) == this);

        SyntaxNode *n = root->clone();
        n->setDepth(root->getDepth() + 1);
        SyntaxNode *nBody = m_body->clone();
        nBody->ignoreGoto();
        n = n->replace(m_body, nBody);
        successors.push_back(n);
    }

    m_body->addSuccessors(root, successors);
}


SyntaxNode *PostTestedLoopSyntaxNode::clone()
{
    PostTestedLoopSyntaxNode *b = new PostTestedLoopSyntaxNode();

    b->m_correspond = this;
    b->m_bb        = m_bb;
    b->m_cond         = m_cond->clone();
    b->m_body        = m_body->clone();
    return b;
}


SyntaxNode *PostTestedLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
    assert(m_correspond != from);

    if (m_body->getCorrespond() == from) {
        assert(to);
        m_body = to;
    }
    else {
        m_body = m_body->replace(from, to);
    }

    return this;
}


SyntaxNode *PostTestedLoopSyntaxNode::findNodeFor(BasicBlock *bb)
{
    if (m_bb == bb) {
        return this;
    }

    SyntaxNode *n = m_body->findNodeFor(bb);

    if (n == m_body) {
        return this;
    }

    return n;
}


void PostTestedLoopSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << "[label=\"loop posttested ";
    os << m_cond << " \"];" << '\n';
    m_body->printAST(root, os);
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << m_body->getNumber() << ";" << '\n';
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << getOutEdge(root, 0)->getNumber() << " [style=dotted];" << '\n';
}


InfiniteLoopSyntaxNode::InfiniteLoopSyntaxNode()
    : m_body(nullptr)
{
}


InfiniteLoopSyntaxNode::~InfiniteLoopSyntaxNode()
{
    if (m_body) {
        delete m_body;
    }
}


int InfiniteLoopSyntaxNode::evaluate(SyntaxNode *root)
{
    int n = 1;

    n += m_body->evaluate(root);
    return n;
}


void InfiniteLoopSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
    // we can always ignore gotos at the end of the body.
    if ((m_body->getNumOutEdges() == 1) && m_body->endsWithGoto()) {
        LOG_VERBOSE("Successor: ignoring goto at end of body of infinite loop");
        assert(m_body->getOutEdge(root, 0) == this);

        SyntaxNode *n = root->clone();
        n->setDepth(root->getDepth() + 1);
        SyntaxNode *nBody = m_body->clone();
        nBody->ignoreGoto();
        n = n->replace(m_body, nBody);
        successors.push_back(n);
    }

    m_body->addSuccessors(root, successors);
}


SyntaxNode *InfiniteLoopSyntaxNode::clone()
{
    InfiniteLoopSyntaxNode *b = new InfiniteLoopSyntaxNode();

    b->m_correspond = this;
    b->m_bb        = m_bb;
    b->m_body        = m_body->clone();
    return b;
}


SyntaxNode *InfiniteLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
    assert(m_correspond != from);

    if (m_body->getCorrespond() == from) {
        assert(to);
        m_body = to;
    }
    else {
        m_body = m_body->replace(from, to);
    }

    return this;
}


SyntaxNode *InfiniteLoopSyntaxNode::findNodeFor(BasicBlock *bb)
{
    if (m_bb == bb) {
        return this;
    }

    SyntaxNode *n = m_body->findNodeFor(bb);

    if (n == m_body) {
        return this;
    }

    return n;
}


void InfiniteLoopSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << "[label=\"loop infinite\"];" << '\n';

    if (m_body) {
        m_body->printAST(root, os);
    }

    if (m_body) {
        os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
        os << " -> " << m_body->getNumber() << ";" << '\n';
    }
}
