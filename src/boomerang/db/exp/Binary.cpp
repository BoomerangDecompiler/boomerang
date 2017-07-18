#include "Binary.h"

#include "boomerang/db/exp/Const.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/TypeVal.h"
#include "boomerang/db/visitor.h"

#include <QRegularExpression>


int tlstrchr(const QString& str, char ch)
{
    static QMap<QChar, QChar> braces { {
                                           '[', ']'
                                       }, {
                                           '{', '}'
                                       }, {
                                           '(', ')'
                                       }
    };
    int i = 0, e = str.length();

    for ( ; i < e; ++i) {
        if (str[i].toLatin1() == ch) {
            return i;
        }

        if (braces.contains(str[i])) {
            QChar end_brace = braces[str[i]];
            ++i; // from next char

            for ( ; i < e; ++i) {
                if (str[i] == end_brace) {
                    break;
                }
            }
        }
    }

    if (i == e) {
        return -1;
    }

    return i;
}


Binary::Binary(OPER op)
    : Unary(op)
{
    // Initialise the 2nd pointer. The first pointer is initialised in the Unary constructor
    // subExp2 = 0;
}


Binary::Binary(OPER op, SharedExp e1, SharedExp e2)
    : Unary(op, e1)
    , subExp2(e2)
{
    assert(subExp1 && subExp2);
}


Binary::Binary(const Binary& o)
    : Unary(m_oper)
{
    setSubExp1(subExp1->clone());
    subExp2 = o.subExp2->clone();
    assert(subExp1 && subExp2);
}


Binary::~Binary()
{
    if (subExp2 != nullptr) {
        // delete subExp2;
    }

    // Note that the first pointer is destructed in the Exp1 destructor
}


void Binary::setSubExp2(SharedExp e)
{
    if (subExp2 != nullptr) {
        // delete subExp2;
    }

    subExp2 = e;
    assert(subExp1 && subExp2);
}


SharedExp Binary::getSubExp2()
{
    assert(subExp1 && subExp2);
    return subExp2;
}


SharedExp& Binary::refSubExp2()
{
    assert(subExp1 && subExp2);
    return subExp2;
}


void Binary::commute()
{
    std::swap(subExp1, subExp2);
    assert(subExp1 && subExp2);
}


SharedExp Binary::clone() const
{
    assert(subExp1 && subExp2);
    return std::make_shared<Binary>(m_oper, subExp1->clone(), subExp2->clone());
}


bool Binary::operator==(const Exp& o) const
{
    assert(subExp1 && subExp2);

    if (o.getOper() == opWild) {
        return true;
    }

    if (nullptr == dynamic_cast<const Binary *>(&o)) {
        return false;
    }

    if (m_oper != ((Binary&)o).m_oper) {
        return false;
    }

    if (!(*subExp1 == *((Binary&)o).getSubExp1())) {
        return false;
    }

    return *subExp2 == *((Binary&)o).getSubExp2();
}


bool Binary::operator<(const Exp& o) const
{
    assert(subExp1 && subExp2);

    if (m_oper < o.getOper()) {
        return true;
    }

    if (m_oper > o.getOper()) {
        return false;
    }

    if (*subExp1 < *((Binary&)o).getSubExp1()) {
        return true;
    }

    if (*((Binary&)o).getSubExp1() < *subExp1) {
        return false;
    }

    return *subExp2 < *((Binary&)o).getSubExp2();
}


bool Binary::operator*=(const Exp& o) const
{
    assert(subExp1 && subExp2);
    const Exp *other = &o;

    if (o.getOper() == opSubscript) {
        other = o.getSubExp1().get();
    }

    if (other->getOper() == opWild) {
        return true;
    }

    if (m_oper != other->getOper()) {
        return false;
    }

    if (!(*subExp1 *= *other->getSubExp1())) {
        return false;
    }

    return *subExp2 *= *other->getSubExp2();
}


void Binary::printr(QTextStream& os, bool html) const
{
    assert(subExp1 && subExp2);

    // The "r" is for recursive: the idea is that we don't want parentheses at the outer level, but a subexpression
    // (recursed from a higher level), we want the parens (at least for standard infix operators)
    switch (m_oper)
    {
        case opSize:
        case opList: // Otherwise, you get (a, (b, (c, d)))
            // There may be others
            // These are the noparen cases
            print(os, html);
            return;

        default:
            break;
    }

    // Normal case: we want the parens
    os << "(";
    this->print(os, html);
    os << ")";
}


void Binary::print(QTextStream& os, bool html) const
{
    assert(subExp1 && subExp2);
    SharedConstExp p1 = getSubExp1();
    SharedConstExp p2 = getSubExp2();

    // Special cases
    switch (m_oper)
    {
        case opSize:
            // This can still be seen after decoding and before type analysis after m[...]
            // *size* is printed after the expression, even though it comes from the first subexpression
            p2->printr(os, html);
            os << "*";
            p1->printr(os, html);
            os << "*";
            return;

        case opFlagCall:
            // The name of the flag function (e.g. ADDFLAGS) should be enough
            std::static_pointer_cast<const Const>(p1)->printNoQuotes(os);
            os << "( ";
            p2->printr(os, html);
            os << " )";
            return;

        case opExpTable:
        case opNameTable:

            if (m_oper == opExpTable) {
                os << "exptable(";
            }
            else {
                os << "nametable(";
            }

            os << p1 << ", " << p2 << ")";
            return;

        case opList:
            // Because "," is the lowest precedence operator, we don't need printr here.
            // Also, same as UQBT, so easier to test
            p1->print(os, html);

            if (!p2->isNil()) {
                os << ", ";
            }

            p2->print(os, html);
            return;

        case opMemberAccess:
            p1->print(os, html);
            os << ".";
            std::static_pointer_cast<const Const>(p2)->printNoQuotes(os);
            return;

        case opArrayIndex:
            p1->print(os, html);
            os << "[";
            p2->print(os, html);
            os << "]";
            return;

        default:
            break;
    }

    // Ordinary infix operators. Emit parens around the binary
    if (p1 == nullptr) {
        os << "<nullptr>";
    }
    else {
        p1->printr(os, html);
    }

    switch (m_oper)
    {
        case opPlus:
            os << " + ";
            break;

        case opMinus:
            os << " - ";
            break;

        case opMult:
            os << " * ";
            break;

        case opMults:
            os << " *! ";
            break;

        case opDiv:
            os << " / ";
            break;

        case opDivs:
            os << " /! ";
            break;

        case opMod:
            os << " % ";
            break;

        case opMods:
            os << " %! ";
            break;

        case opFPlus:
            os << " +f ";
            break;

        case opFMinus:
            os << " -f ";
            break;

        case opFMult:
            os << " *f ";
            break;

        case opFDiv:
            os << " /f ";
            break;

        case opPow:
            os << " pow ";
            break; // Raising to power

        case opAnd:
            os << " and ";
            break;

        case opOr:
            os << " or ";
            break;

        case opBitAnd:
            os << " & ";
            break;

        case opBitOr:
            os << " | ";
            break;

        case opBitXor:
            os << " ^ ";
            break;

        case opEquals:
            os << " = ";
            break;

        case opNotEqual:
            os << " ~= ";
            break;

        case opLess:

            if (html) {
                os << " &lt; ";
            }
            else {
                os << " < ";
            }

            break;

        case opGtr:

            if (html) {
                os << " &gt; ";
            }
            else {
                os << " > ";
            }

            break;

        case opLessEq:

            if (html) {
                os << " &lt;= ";
            }
            else {
                os << " <= ";
            }

            break;

        case opGtrEq:

            if (html) {
                os << " &gt;= ";
            }
            else {
                os << " >= ";
            }

            break;

        case opLessUns:

            if (html) {
                os << " &lt;u ";
            }
            else {
                os << " <u ";
            }

            break;

        case opGtrUns:

            if (html) {
                os << " &gt;u ";
            }
            else {
                os << " >u ";
            }

            break;

        case opLessEqUns:

            if (html) {
                os << " &lt;u ";
            }
            else {
                os << " <=u ";
            }

            break;

        case opGtrEqUns:

            if (html) {
                os << " &gt;=u ";
            }
            else {
                os << " >=u ";
            }

            break;

        case opUpper:
            os << " GT ";
            break;

        case opLower:
            os << " LT ";
            break;

        case opShiftL:

            if (html) {
                os << " &lt;&lt; ";
            }
            else {
                os << " << ";
            }

            break;

        case opShiftR:

            if (html) {
                os << " &gt;&gt; ";
            }
            else {
                os << " >> ";
            }

            break;

        case opShiftRA:

            if (html) {
                os << " &gt;&gt;A ";
            }
            else {
                os << " >>A ";
            }

            break;

        case opRotateL:
            os << " rl ";
            break;

        case opRotateR:
            os << " rr ";
            break;

        case opRotateLC:
            os << " rlc ";
            break;

        case opRotateRC:
            os << " rrc ";
            break;

        default:
            LOG << "Binary::print invalid operator " << operToString(m_oper) << "\n";
            assert(false);
    }

    if (p2 == nullptr) {
        os << "<nullptr>";
    }
    else {
        p2->printr(os, html);
    }
}


void Binary::appendDotFile(QTextStream& of)
{
    // First a node for this Binary object
    of << "e_" << HostAddress(this) << " [shape=record,label=\"{";
    of << operToString(m_oper) << "\\n" << HostAddress(this) << " | ";
    of << "{<p1> | <p2>}";
    of << " }\"];\n";
    subExp1->appendDotFile(of);
    subExp2->appendDotFile(of);
    // Now an edge for each subexpression
    of << "e_" << HostAddress(this) << ":p1->e_" << HostAddress(subExp1.get()) << ";\n";
    of << "e_" << HostAddress(this) << ":p2->e_" << HostAddress(subExp2.get()) << ";\n";
}


SharedExp Binary::match(const SharedConstExp& pattern)
{
    assert(subExp1 && subExp2);

    if (m_oper != pattern->getOper()) {
        return Exp::match(pattern);
    }

    SharedExp b_lhs = subExp1->match(pattern->getSubExp1());

    if (b_lhs == nullptr) {
        return nullptr;
    }

    SharedExp b_rhs = subExp2->match(pattern->getSubExp2());

    if (b_rhs == nullptr) {
        return nullptr;
    }

    if (b_lhs->getOper() == opNil) {
        return b_rhs;
    }

    if (b_rhs->getOper() == opNil) {
        return b_lhs;
    }

    SharedExp result = Terminal::get(opNil);

    // TODO: verify that adding (l &&) is not violating unwritten validity assertion
    for (SharedExp l = b_lhs; l && (l->getOper() != opNil); l = l->getSubExp2()) {
        for (SharedExp r = b_rhs; r && r->getOper() != opNil; r = r->getSubExp2()) {
            if ((*l->getSubExp1()->getSubExp1() == *r->getSubExp1()->getSubExp1()) &&
                !(*l->getSubExp1()->getSubExp2() == *r->getSubExp1()->getSubExp2())) {

                return nullptr; // must be agreement between LHS and RHS
                }
                else {
                    result = Binary::get(opList, l->getSubExp1()->clone(), result);
                }
        }
    }

    for (SharedExp r = b_rhs; r->getOper() != opNil; r = r->getSubExp2()) {
        result = Binary::get(opList, r->getSubExp1()->clone(), result);
    }

    return result;
}



bool Binary::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
    if (Exp::match(pattern, bindings)) {
        return true;
    }

#ifdef DEBUG_MATCH
    LOG << "binary::match " << this << " to " << pattern << ".\n";
#endif

    if ((m_oper == opMemberAccess) && (-1 != tlstrchr(pattern, '.'))) {
        QString sub1        = pattern;
        int     split_point = tlstrchr(sub1, '.');
        QString follow      = sub1.right(sub1.length() - split_point);
        sub1 = sub1.left(split_point);

        if (subExp1->match(sub1, bindings)) {
            assert(subExp2->isStrConst());

            if (follow == std::static_pointer_cast<const Const>(subExp2)->getStr()) {
                return true;
            }

            if ((follow.lastIndexOf(QRegularExpression("[a-zA-Z0-9]+")) == 0)) {
                bindings[follow] = subExp2;
                return true;
            }
        }
    }

    if (m_oper == opArrayIndex) {
        if (!pattern.endsWith(']')) {
            return false;
        }

        QString sub1 = pattern;
        QString sub2 = sub1.mid(sub1.lastIndexOf('[') + 1);

        if (subExp1->match(sub1, bindings) && subExp2->match(sub2, bindings)) {
            return true;
        }
    }

    if ((m_oper == opPlus) && (-1 != tlstrchr(pattern, '+'))) {
        int     splitpoint = tlstrchr(pattern, '+');
        QString sub1       = pattern.left(splitpoint);
        QString sub2       = pattern.mid(splitpoint + 1).trimmed();

        if (subExp1->match(sub1, bindings) && subExp2->match(sub2, bindings)) {
            return true;
        }
    }

    if ((m_oper == opMinus) && (-1 != tlstrchr(pattern, '-'))) {
        int     splitpoint = tlstrchr(pattern, '-');
        QString sub1       = pattern.left(splitpoint);
        QString sub2       = pattern.mid(splitpoint + 1).trimmed();

        if (subExp1->match(sub1, bindings) && subExp2->match(sub2, bindings)) {
            return true;
        }
    }

    return false;
}


void Binary::doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once)
{
    assert(subExp1 && subExp2);
    doSearch(search, subExp1, li, once);

    if (once && li.size()) {
        return;
    }

    doSearch(search, subExp2, li, once);
}


SharedExp Binary::simplifyArith()
{
    assert(subExp1 && subExp2);
    subExp1 = subExp1->simplifyArith(); // FIXME: does this make sense?
    subExp2 = subExp2->simplifyArith(); // FIXME: ditto

    if ((m_oper != opPlus) && (m_oper != opMinus)) {
        return shared_from_this();
    }

    // Partition this expression into positive non-integer terms, negative
    // non-integer terms and integer terms.
    std::list<SharedExp> positives;
    std::list<SharedExp> negatives;
    std::vector<int>     integers;
    partitionTerms(positives, negatives, integers, false);

    // Now reduce these lists by cancelling pairs
    // Note: can't improve this algorithm using multisets, since can't instantiate multisets of type Exp (only Exp*).
    // The Exp* in the multisets would be sorted by address, not by value of the expression.
    // So they would be unsorted, same as lists!
    std::list<SharedExp>::iterator pp = positives.begin();
    std::list<SharedExp>::iterator nn = negatives.begin();

    while (pp != positives.end()) {
        bool inc = true;

        while (nn != negatives.end()) {
            if (**pp == **nn) {
                // A positive and a negative that are equal; therefore they cancel
                pp  = positives.erase(pp); // Erase the pointers, not the Exps
                nn  = negatives.erase(nn);
                inc = false;               // Don't increment pp now
                break;
            }

            nn++;
        }

        if (pp == positives.end()) {
            break;
        }

        if (inc) {
            pp++;
        }
    }

    // Summarise the set of integers to a single number.
    int sum = std::accumulate(integers.begin(), integers.end(), 0);

    // Now put all these elements back together and return the result
    if (positives.size() == 0) {
        if (negatives.size() == 0) {
            return Const::get(sum);
        }
        else {
            // No positives, some negatives. sum - Acc
            return Binary::get(opMinus, Const::get(sum), Exp::accumulate(negatives));
        }
    }

    if (negatives.size() == 0) {
        // Positives + sum
        if (sum == 0) {
            // Just positives
            return Exp::accumulate(positives);
        }
        else {
            OPER _op = opPlus;

            if (sum < 0) {
                _op = opMinus;
                sum = -sum;
            }

            return Binary::get(_op, Exp::accumulate(positives), Const::get(sum));
        }
    }

    // Some positives, some negatives
    if (sum == 0) {
        // positives - negatives
        return Binary::get(opMinus, Exp::accumulate(positives), Exp::accumulate(negatives));
    }

    // General case: some positives, some negatives, a sum
    OPER _op = opPlus;

    if (sum < 0) {
        _op = opMinus; // Return (pos - negs) - sum
        sum = -sum;
    }

    return Binary::get(_op, Binary::get(opMinus, Exp::accumulate(positives), Exp::accumulate(negatives)),
                       Const::get(sum));
}



SharedExp Binary::polySimplify(bool& bMod)
{
    assert(subExp1 && subExp2);

    SharedExp res = shared_from_this();

    subExp1 = subExp1->polySimplify(bMod);
    subExp2 = subExp2->polySimplify(bMod);

    OPER opSub1 = subExp1->getOper();
    OPER opSub2 = subExp2->getOper();

    if ((opSub1 == opIntConst) && (opSub2 == opIntConst)) {
        // k1 op k2, where k1 and k2 are integer constants
        int  k1     = std::static_pointer_cast<Const>(subExp1)->getInt();
        int  k2     = std::static_pointer_cast<Const>(subExp2)->getInt();
        bool change = true;

        switch (m_oper)
        {
            case opPlus:
                k1 = k1 + k2;
                break;

            case opMinus:
                k1 = k1 - k2;
                break;

            case opDiv:
                k1 = (int)((unsigned)k1 / (unsigned)k2);
                break;

            case opDivs:
                k1 = k1 / k2;
                break;

            case opMod:
                k1 = (int)((unsigned)k1 % (unsigned)k2);
                break;

            case opMods:
                k1 = k1 % k2;
                break;

            case opMult:
                k1 = (int)((unsigned)k1 * (unsigned)k2);
                break;

            case opMults:
                k1 = k1 * k2;
                break;

            case opShiftL:

                if (k2 >= 32) {
                    k1 = 0;
                }
                else {
                    k1 = k1 << k2;
                }

                break;

            case opShiftR:
                k1 = k1 >> k2;
                break;

            case opShiftRA:
                k1 = (k1 >> k2) | (((1 << k2) - 1) << (32 - k2));
                break;

            case opBitOr:
                k1 = k1 | k2;
                break;

            case opBitAnd:
                k1 = k1 & k2;
                break;

            case opBitXor:
                k1 = k1 ^ k2;
                break;

            case opEquals:
                k1 = (k1 == k2);
                break;

            case opNotEqual:
                k1 = (k1 != k2);
                break;

            case opLess:
                k1 = (k1 < k2);
                break;

            case opGtr:
                k1 = (k1 > k2);
                break;

            case opLessEq:
                k1 = (k1 <= k2);
                break;

            case opGtrEq:
                k1 = (k1 >= k2);
                break;

            case opLessUns:
                k1 = ((unsigned)k1 < (unsigned)k2);
                break;

            case opGtrUns:
                k1 = ((unsigned)k1 > (unsigned)k2);
                break;

            case opLessEqUns:
                k1 = ((unsigned)k1 <= (unsigned)k2);
                break;

            case opGtrEqUns:
                k1 = ((unsigned)k1 >= (unsigned)k2);
                break;

            default:
                change = false;
        }

        if (change) {
            res  = Const::get(k1);
            bMod = true;
            return res;
        }
    }

    if (((m_oper == opBitXor) || (m_oper == opMinus)) && (*subExp1 == *subExp2)) {
        // x ^ x or x - x: result is zero
        res  = Const::get(0);
        bMod = true;
        return res;
    }

    if (((m_oper == opBitOr) || (m_oper == opBitAnd)) && (*subExp1 == *subExp2)) {
        // x | x or x & x: result is x
        res  = subExp1;
        bMod = true;
        return res;
    }

    if ((m_oper == opEquals) && (*subExp1 == *subExp2)) {
        // x == x: result is true
        // delete this;
        res  = std::make_shared<Terminal>(opTrue);
        bMod = true;
        return res;
    }

    // Might want to commute to put an integer constant on the RHS
    // Later simplifications can rely on this (ADD other ops as necessary)
    if ((opSub1 == opIntConst) && ((m_oper == opPlus) || (m_oper == opMult) || (m_oper == opMults) || (m_oper == opBitOr) || (m_oper == opBitAnd))) {
        commute();
        // Swap opSub1 and opSub2 as well
        std::swap(opSub1, opSub2);
        // This is not counted as a modification
    }

    // Similarly for boolean constants
    if (subExp1->isBoolConst() && !subExp2->isBoolConst() && ((m_oper == opAnd) || (m_oper == opOr))) {
        commute();
        // Swap opSub1 and opSub2 as well
        std::swap(opSub1, opSub2);
        // This is not counted as a modification
    }

    // Similarly for adding stuff to the addresses of globals
    if (subExp2->isAddrOf() && subExp2->getSubExp1()->isSubscript() &&
        subExp2->getSubExp1()->getSubExp1()->isGlobal() && (m_oper == opPlus)) {
        commute();
    // Swap opSub1 and opSub2 as well
    std::swap(opSub1, opSub2);
    // This is not counted as a modification
        }

        // check for (x + a) + b where a and b are constants, becomes x + a+b
        if ((m_oper == opPlus) && (opSub1 == opPlus) && (opSub2 == opIntConst) && (subExp1->getSubExp2()->getOper() == opIntConst)) {
            int n = std::static_pointer_cast<Const>(subExp2)->getInt();
            res = res->getSubExp1();
            std::shared_ptr<Const> c_subexp(std::static_pointer_cast<Const>(res->getSubExp2()));
            c_subexp->setInt(c_subexp->getInt() + n);
            bMod = true;
            return res;
        }

        // check for (x - a) + b where a and b are constants, becomes x + -a+b
        if ((m_oper == opPlus) && (opSub1 == opMinus) && (opSub2 == opIntConst) && (subExp1->getSubExp2()->getOper() == opIntConst)) {
            int n = std::static_pointer_cast<Const>(subExp2)->getInt();
            res = res->getSubExp1();
            res->setOper(opPlus);
            std::shared_ptr<Const> c_subexp(std::static_pointer_cast<Const>(res->getSubExp2()));
            c_subexp->setInt(-c_subexp->getInt() + n);
            bMod = true;
            return res;
        }

        // check for (x * k) - x, becomes x * (k-1)
        // same with +
        if (((m_oper == opMinus) || (m_oper == opPlus)) && ((opSub1 == opMults) || (opSub1 == opMult)) &&
            (*subExp2 == *subExp1->getSubExp1())) {
            res = res->getSubExp1();
        res->setSubExp2(Binary::get(m_oper, res->getSubExp2(), Const::get(1)));
        bMod = true;
        return res;
            }

            // check for x + (x * k), becomes x * (k+1)
            if ((m_oper == opPlus) && ((opSub2 == opMults) || (opSub2 == opMult)) && (*subExp1 == *subExp2->getSubExp1())) {
                res = res->getSubExp2();
                res->setSubExp2(Binary::get(opPlus, res->getSubExp2(), Const::get(1)));
                bMod = true;
                return res;
            }

            // Turn a + -K into a - K (K is int const > 0)
            // Also a - -K into a + K (K is int const > 0)
            // Does not count as a change
            if (((m_oper == opPlus) || (m_oper == opMinus)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() < 0)) {
                std::static_pointer_cast<Const>(subExp2)->setInt(-std::static_pointer_cast<const Const>(subExp2)->getInt());
                m_oper = m_oper == opPlus ? opMinus : opPlus;
            }

            // Check for exp + 0  or  exp - 0  or  exp | 0
            if (((m_oper == opPlus) || (m_oper == opMinus) || (m_oper == opBitOr)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0)) {
                res  = res->getSubExp1();
                bMod = true;
                return res;
            }

            // Check for exp or false
            if ((m_oper == opOr) && subExp2->isFalse()) {
                res  = res->getSubExp1();
                bMod = true;
                return res;
            }

            // Check for SharedExp 0  or exp & 0
            if (((m_oper == opMult) || (m_oper == opMults) || (m_oper == opBitAnd)) && (opSub2 == opIntConst) &&
                (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0)) {
                // delete res;
                res  = Const::get(0);
            bMod = true;
            return res;
                }

                // Check for exp and false
                if ((m_oper == opAnd) && subExp2->isFalse()) {
                    // delete res;
                    res  = Terminal::get(opFalse);
                    bMod = true;
                    return res;
                }

                // Check for SharedExp 1
                if (((m_oper == opMult) || (m_oper == opMults)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 1)) {
                    res  = res->getSubExp1();
                    bMod = true;
                    return res;
                }

                // Check for SharedExp x / x
                if (((m_oper == opDiv) || (m_oper == opDivs)) && ((opSub1 == opMult) || (opSub1 == opMults)) &&
                    (*subExp2 == *subExp1->getSubExp2())) {
                    res  = res->getSubExp1();
                res  = res->getSubExp1();
                bMod = true;
                return res;
                    }

                    // Check for exp / 1, becomes exp
                    if (((m_oper == opDiv) || (m_oper == opDivs)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 1)) {
                        res  = res->getSubExp1();
                        bMod = true;
                        return res;
                    }

                    // Check for exp % 1, becomes 0
                    if (((m_oper == opMod) || (m_oper == opMods)) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 1)) {
                        res  = Const::get(0);
                        bMod = true;
                        return res;
                    }

                    // Check for SharedExp x % x, becomes 0
                    if (((m_oper == opMod) || (m_oper == opMods)) && ((opSub1 == opMult) || (opSub1 == opMults)) &&
                        (*subExp2 == *subExp1->getSubExp2())) {
                        res  = Const::get(0);
                    bMod = true;
                    return res;
                        }

                        // Check for exp AND -1 (bitwise AND)
                        if ((m_oper == opBitAnd) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == -1)) {
                            res  = res->getSubExp1();
                            bMod = true;
                            return res;
                        }

                        // Check for exp AND TRUE (logical AND)
                        if ((m_oper == opAnd) &&
                            // Is the below really needed?
                            ((((opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() != 0))) || subExp2->isTrue())) {
                            res  = res->getSubExp1();
                        bMod = true;
                        return res;
                            }

                            // Check for exp OR TRUE (logical OR)
                            if ((m_oper == opOr) && ((((opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() != 0))) || subExp2->isTrue())) {
                                // delete res;
                                res  = Terminal::get(opTrue);
                                bMod = true;
                                return res;
                            }

                            // Check for [exp] << k where k is a positive integer const

                            if ((m_oper == opShiftL) && (opSub2 == opIntConst)) {
                                int k = std::static_pointer_cast<const Const>(subExp2)->getInt();
                                if ((k >= 0) && (k < 32)) {
                                    res->setOper(opMult);
                                    std::static_pointer_cast<Const>(subExp2)->setInt(1 << k);
                                    bMod = true;
                                    return res;
                                }
                            }

                            if ((m_oper == opShiftR) && (opSub2 == opIntConst)) {
                                int k = std::static_pointer_cast<const Const>(subExp2)->getInt();
                                if ((k >= 0) && (k < 32)) {
                                    res->setOper(opDiv);
                                    std::static_pointer_cast<Const>(subExp2)->setInt(1 << k);
                                    bMod = true;
                                    return res;
                                }
                            }

                            /*
                             *  // Check for -x compare y, becomes x compare -y
                             *  // doesn't count as a change
                             *  if (    isComparison() &&
                             *                  opSub1 == opNeg) {
                             *          SharedExp e = subExp1;
                             *          subExp1 = e->getSubExp1()->clone();
                             *          ;//delete e;
                             *          subExp2 = Unary::get(opNeg, subExp2);
                             *  }
                             *
                             *  // Check for (x + y) compare 0, becomes x compare -y
                             *  if (    isComparison() &&
                             *                  opSub2 == opIntConst && ((Const*)subExp2)->getInt() == 0 &&
                             *                  opSub1 == opPlus) {
                             *          ;//delete subExp2;
                             *          Binary *b = (Binary*)subExp1;
                             *          subExp2 = b->subExp2;
                             *          b->subExp2 = 0;
                             *          subExp1 = b->subExp1;
                             *          b->subExp1 = 0;
                             *          ;//delete b;
                             *          subExp2 = Unary::get(opNeg, subExp2);
                             *          bMod = true;
                             *          return res;
                             *  }
                             */
                            // Check for (x == y) == 1, becomes x == y
                            if ((m_oper == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 1) && (opSub1 == opEquals)) {
                                auto b = std::static_pointer_cast<Binary>(subExp1);
                                subExp2 = std::move(b->subExp2);
                                subExp1 = std::move(b->subExp1);
                                bMod    = true;
                                return res;
                            }

                            // Check for x + -y == 0, becomes x == y
                            if ((m_oper == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opPlus) &&
                                (subExp1->getSubExp2()->getOper() == opIntConst)) {
                                auto b = std::static_pointer_cast<Binary>(subExp1);
                            int  n = std::static_pointer_cast<Const>(b->subExp2)->getInt();

                            if (n < 0) {
                                subExp2 = std::move(b->subExp2);
                                std::static_pointer_cast<Const>(subExp2)->setInt(-std::static_pointer_cast<const Const>(subExp2)->getInt());
                                subExp1 = std::move(b->subExp1);
                                bMod    = true;
                                return res;
                            }
                                }

                                // Check for (x == y) == 0, becomes x != y
                                if ((m_oper == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opEquals)) {
                                    auto b = std::static_pointer_cast<Binary>(subExp1);
                                    subExp2 = std::move(b->subExp2);
                                    subExp1 = std::move(b->subExp1);
                                    bMod    = true;
                                    res->setOper(opNotEqual);
                                    return res;
                                }

                                // Check for (x == y) != 1, becomes x != y
                                if ((m_oper == opNotEqual) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 1) && (opSub1 == opEquals)) {
                                    auto b = std::static_pointer_cast<Binary>(subExp1);
                                    subExp2 = std::move(b->subExp2);
                                    subExp1 = std::move(b->subExp1);
                                    bMod    = true;
                                    res->setOper(opNotEqual);
                                    return res;
                                }

                                // Check for (x == y) != 0, becomes x == y
                                if ((m_oper == opNotEqual) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opEquals)) {
                                    res  = res->getSubExp1();
                                    bMod = true;
                                    return res;
                                }

                                // Check for (0 - x) != 0, becomes x != 0
                                if ((m_oper == opNotEqual) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opMinus) &&
                                    subExp1->getSubExp1()->isIntConst() && (std::static_pointer_cast<const Const>(subExp1->getSubExp1())->getInt() == 0)) {
                                    res  = Binary::get(opNotEqual, subExp1->getSubExp2()->clone(), subExp2->clone());
                                bMod = true;
                                return res;
                                    }

                                    // Check for (x > y) == 0, becomes x <= y
                                    if ((m_oper == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opGtr)) {
                                        auto b = std::static_pointer_cast<Binary>(subExp1);
                                        subExp2 = std::move(b->subExp2);
                                        subExp1 = std::move(b->subExp1);
                                        bMod    = true;
                                        res->setOper(opLessEq);
                                        return res;
                                    }

                                    // Check for (x >u y) == 0, becomes x <=u y
                                    if ((m_oper == opEquals) && (opSub2 == opIntConst) && (std::static_pointer_cast<const Const>(subExp2)->getInt() == 0) && (opSub1 == opGtrUns)) {
                                        auto b = std::static_pointer_cast<Binary>(subExp1);
                                        subExp2 = std::move(b->subExp2);
                                        subExp1 = std::move(b->subExp1);
                                        bMod    = true;
                                        res->setOper(opLessEqUns);
                                        return res;
                                    }

                                    auto b1 = std::dynamic_pointer_cast<Binary>(subExp1);
                                    auto b2 = std::dynamic_pointer_cast<Binary>(subExp2);

                                    // Check for (x <= y) || (x == y), becomes x <= y
                                    if ((m_oper == opOr) && (opSub2 == opEquals) &&
                                        ((opSub1 == opGtrEq) || (opSub1 == opLessEq) || (opSub1 == opGtrEqUns) || (opSub1 == opLessEqUns)) &&
                                        (((*b1->subExp1 == *b2->subExp1) && (*b1->subExp2 == *b2->subExp2)) ||
                                        ((*b1->subExp1 == *b2->subExp2) && (*b1->subExp2 == *b2->subExp1)))) {
                                        res  = res->getSubExp1();
                                    bMod = true;
                                    return res;
                                        }

                                        // For (a || b) or (a && b) recurse on a and b
                                        if ((m_oper == opOr) || (m_oper == opAnd)) {
                                            subExp1 = subExp1->polySimplify(bMod);
                                            subExp2 = subExp2->polySimplify(bMod);
                                            return res;
                                        }

                                        // check for (x & x), becomes x
                                        if ((m_oper == opBitAnd) && (*subExp1 == *subExp2)) {
                                            res  = res->getSubExp1();
                                            bMod = true;
                                            return res;
                                        }

                                        // check for a + a*n, becomes a*(n+1) where n is an int
                                        if ((m_oper == opPlus) && (opSub2 == opMult) && (*subExp1 == *subExp2->getSubExp1()) &&
                                            (subExp2->getSubExp2()->getOper() == opIntConst)) {
                                            res = res->getSubExp2();
                                        res->access<Const, 2>()->setInt(res->access<Const, 2>()->getInt() + 1);
                                        bMod = true;
                                        return res;
                                            }

                                            // check for a*n*m, becomes a*(n*m) where n and m are ints
                                            if ((m_oper == opMult) && (opSub1 == opMult) && (opSub2 == opIntConst) && (subExp1->getSubExp2()->getOper() == opIntConst)) {
                                                int m = std::static_pointer_cast<const Const>(subExp2)->getInt();
                                                res = res->getSubExp1();
                                                res->access<Const, 2>()->setInt(res->access<Const, 2>()->getInt() * m);
                                                bMod = true;
                                                return res;
                                            }

                                            // check for !(a == b) becomes a != b
                                            if ((m_oper == opLNot) && (opSub1 == opEquals)) {
                                                res = res->getSubExp1();
                                                res->setOper(opNotEqual);
                                                bMod = true;
                                                return res;
                                            }

                                            // check for !(a != b) becomes a == b
                                            if ((m_oper == opLNot) && (opSub1 == opNotEqual)) {
                                                res = res->getSubExp1();
                                                res->setOper(opEquals);
                                                bMod = true;
                                                return res;
                                            }

                                            // FIXME: suspect this was only needed for ADHOC TA
                                            // check for exp + n where exp is a pointer to a compound type
                                            // becomes &m[exp].m + r where m is the member at offset n and r is n - the offset to member m
                                            SharedType ty = nullptr; // Type of subExp1

                                            if (subExp1->isSubscript()) {
                                                const Instruction *def = std::static_pointer_cast<RefExp>(subExp1)->getDef();

                                                if (def) {
                                                    ty = def->getTypeFor(subExp1->getSubExp1());
                                                }
                                            }

                                            if ((m_oper == opPlus) && ty && ty->resolvesToPointer() && ty->as<PointerType>()->getPointsTo()->resolvesToCompound() &&
                                                (opSub2 == opIntConst)) {
                                                unsigned n = (unsigned)std::static_pointer_cast<const Const>(subExp2)->getInt();
                                            std::shared_ptr<CompoundType> c = ty->as<PointerType>()->getPointsTo()->as<CompoundType>();
                                            res = convertFromOffsetToCompound(subExp1, c, n);

                                            if (res) {
                                                LOG_VERBOSE(1) << "(trans1) replacing " << shared_from_this() << " with " << res << "\n";
                                                bMod = true;
                                                return res;
                                            }
                                                }

                                                if ((m_oper == opFMinus) && (subExp1->getOper() == opFltConst) && (std::static_pointer_cast<const Const>(subExp1)->getFlt() == 0.0)) {
                                                    res  = Unary::get(opFNeg, subExp2);
                                                    bMod = true;
                                                    return res;
                                                }

                                                if (((m_oper == opPlus) || (m_oper == opMinus)) && ((subExp1->getOper() == opMults) || (subExp1->getOper() == opMult)) &&
                                                    (subExp2->getOper() == opIntConst) && (subExp1->getSubExp2()->getOper() == opIntConst)) {
                                                    int n1 = std::static_pointer_cast<const Const>(subExp2)->getInt();
                                                int n2 = subExp1->access<Const, 2>()->getInt();

                                                if (n1 == n2) {
                                                    res = Binary::get(subExp1->getOper(), Binary::get(m_oper, subExp1->getSubExp1()->clone(), Const::get(1)),
                                                                      Const::get(n1));
                                                    bMod = true;
                                                    return res;
                                                }
                                                    }

                                                    if (((m_oper == opPlus) || (m_oper == opMinus)) && (subExp1->getOper() == opPlus) && (subExp2->getOper() == opIntConst) &&
                                                        ((subExp1->getSubExp2()->getOper() == opMults) || (subExp1->getSubExp2()->getOper() == opMult)) &&
                                                        (subExp1->access<Exp, 2, 2>()->getOper() == opIntConst)) {
                                                        int n1 = std::static_pointer_cast<const Const>(subExp2)->getInt();
                                                    int n2 = subExp1->access<Const, 2, 2>()->getInt();

                                                    if (n1 == n2) {
                                                        res = Binary::get(opPlus, subExp1->getSubExp1(),
                                                                          Binary::get(subExp1->getSubExp2()->getOper(),
                                                                                      Binary::get(m_oper, subExp1->access<Exp, 2, 1>()->clone(), Const::get(1)),
                                                                                      Const::get(n1)));
                                                        bMod = true;
                                                        return res;
                                                    }
                                                        }

                                                        // check for ((x * a) + (y * b)) / c where a, b and c are all integers and a and b divide evenly by c
                                                        // becomes: (x * a/c) + (y * b/c)
                                                        if ((m_oper == opDiv) && (subExp1->getOper() == opPlus) && (subExp2->getOper() == opIntConst) &&
                                                            (subExp1->getSubExp1()->getOper() == opMult) && (subExp1->getSubExp2()->getOper() == opMult) &&
                                                            (subExp1->access<Exp, 1, 2>()->getOper() == opIntConst) &&
                                                            (subExp1->access<Exp, 2, 2>()->getOper() == opIntConst)) {
                                                            int a = subExp1->access<Const, 1, 2>()->getInt();
                                                        int b = subExp1->access<Const, 2, 2>()->getInt();
                                                        int c = std::static_pointer_cast<const Const>(subExp2)->getInt();

                                                        if (((a % c) == 0) && ((b % c) == 0)) {
                                                            res = Binary::get(opPlus, Binary::get(opMult, subExp1->getSubExp1()->getSubExp1(), Const::get(a / c)),
                                                                              Binary::get(opMult, subExp1->access<Exp, 2, 1>(), Const::get(b / c)));
                                                            bMod = true;
                                                            return res;
                                                        }
                                                            }

                                                            // check for ((x * a) + (y * b)) % c where a, b and c are all integers
                                                            // becomes: (y * b) % c if a divides evenly by c
                                                            // becomes: (x * a) % c if b divides evenly by c
                                                            // becomes: 0            if both a and b divide evenly by c
                                                            if ((m_oper == opMod) && (subExp1->getOper() == opPlus) && (subExp2->getOper() == opIntConst) &&
                                                                (subExp1->getSubExp1()->getOper() == opMult) && (subExp1->getSubExp2()->getOper() == opMult) &&
                                                                (subExp1->getSubExp1()->getSubExp2()->getOper() == opIntConst) &&
                                                                (subExp1->getSubExp2()->getSubExp2()->getOper() == opIntConst)) {
                                                                int a = subExp1->access<Const, 1, 2>()->getInt();
                                                            int b = subExp1->access<Const, 2, 2>()->getInt();
                                                            int c = std::static_pointer_cast<const Const>(subExp2)->getInt();

                                                            if (((a % c) == 0) && ((b % c) == 0)) {
                                                                res  = Const::get(0);
                                                                bMod = true;
                                                                return res;
                                                            }

                                                            if ((a % c) == 0) {
                                                                res  = Binary::get(opMod, subExp1->getSubExp2()->clone(), Const::get(c));
                                                                bMod = true;
                                                                return res;
                                                            }

                                                            if ((b % c) == 0) {
                                                                res  = Binary::get(opMod, subExp1->getSubExp1()->clone(), Const::get(c));
                                                                bMod = true;
                                                                return res;
                                                            }
                                                                }

                                                                // Check for 0 - (0 <u exp1) & exp2 => exp2
                                                                if ((m_oper == opBitAnd) && (opSub1 == opMinus)) {
                                                                    SharedExp leftOfMinus = subExp1->getSubExp1();

                                                                    if (leftOfMinus->isIntConst() && (std::static_pointer_cast<const Const>(leftOfMinus)->getInt() == 0)) {
                                                                        SharedExp rightOfMinus = subExp1->getSubExp2();

                                                                        if (rightOfMinus->getOper() == opLessUns) {
                                                                            SharedExp leftOfLess = rightOfMinus->getSubExp1();

                                                                            if (leftOfLess->isIntConst() && (std::static_pointer_cast<const Const>(leftOfLess)->getInt() == 0)) {
                                                                                res  = getSubExp2();
                                                                                bMod = true;
                                                                                return res;
                                                                            }
                                                                        }
                                                                    }
                                                                }

                                                                // Replace opSize(n, loc) with loc and set the type if needed
                                                                if ((m_oper == opSize) && subExp2->isLocation()) {
                                                                    res  = res->getSubExp2();
                                                                    bMod = true;
                                                                    return res;
                                                                }

                                                                return res;
}



SharedExp Binary::simplifyAddr()
{
    assert(subExp1 && subExp2);

    subExp1 = subExp1->simplifyAddr();
    subExp2 = subExp2->simplifyAddr();
    return shared_from_this();
}


SharedExp Binary::constrainSub(const std::shared_ptr<TypeVal>& typeVal1, const std::shared_ptr<TypeVal>& typeVal2)
{
    assert(subExp1 && subExp2);

    SharedExp con1 = subExp1->genConstraints(typeVal1);
    SharedExp con2 = subExp2->genConstraints(typeVal2);
    return Binary::get(opAnd, con1, con2);
}


SharedExp Binary::genConstraints(SharedExp result)
{
    assert(subExp1 && subExp2);

    SharedType restrictTo = nullptr;

    if (result->isTypeVal()) {
        restrictTo = result->access<TypeVal>()->getType();
    }

    SharedExp res     = nullptr;
    auto      intType = IntegerType::get(0); // Wild size (=0)
    auto      intVal  = TypeVal::get(intType);

    switch (m_oper)
    {
        case opFPlus:
        case opFMinus:
        case opFMult:
        case opFDiv:
        {
            if (restrictTo && !restrictTo->isFloat()) {
                // Result can only be float
                return Terminal::get(opFalse);
            }

            // MVE: what about sizes?
            auto ft  = FloatType::get();
            auto ftv = TypeVal::get(ft);
            res = constrainSub(ftv, ftv);

            if (!restrictTo) {
                // Also constrain the result
                res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), ftv));
            }

            return res;
        }

        case opBitAnd:
        case opBitOr:
        case opBitXor:
        {
            if (restrictTo && !restrictTo->isInteger()) {
                // Result can only be integer
                return Terminal::get(opFalse);
            }

            // MVE: What about sizes?
            auto it  = IntegerType::get(STD_SIZE, 0);
            auto itv = TypeVal::get(it);
            res = constrainSub(itv, itv);

            if (!restrictTo) {
                // Also constrain the result
                res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), itv));
            }

            return res;
        }

        case opPlus:
        {
            // A pointer to anything
            SharedType ptrType = PointerType::newPtrAlpha();
            auto       ptrVal  = TypeVal::get(ptrType); // Type value of ptr to anything

            if (!restrictTo || restrictTo->isInteger()) {
                // int + int -> int
                res = constrainSub(intVal, intVal);

                if (!restrictTo) {
                    res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), intVal->clone()));
                }
            }

            if (!restrictTo || restrictTo->isPointer()) {
                // ptr + int -> ptr
                SharedExp res2 = constrainSub(ptrVal, intVal);

                if (!restrictTo) {
                    res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), ptrVal->clone()));
                }

                if (res) {
                    res = Binary::get(opOr, res, res2);
                }
                else {
                    res = res2;
                }

                // int + ptr -> ptr
                res2 = constrainSub(intVal, ptrVal);

                if (!restrictTo) {
                    res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), ptrVal->clone()));
                }

                if (res) {
                    res = Binary::get(opOr, res, res2);
                }
                else {
                    res = res2;
                }
            }

            if (res) {
                return res->simplify();
            }

            return Terminal::get(opFalse);
        }

        case opMinus:
        {
            SharedType ptrType = PointerType::newPtrAlpha();
            auto       ptrVal  = TypeVal::get(ptrType);

            if (!restrictTo || restrictTo->isInteger()) {
                // int - int -> int
                res = constrainSub(intVal, intVal);

                if (!restrictTo) {
                    res = Binary::get(opAnd, res, Binary::get(opEquals, result->clone(), intVal->clone()));
                }

                // ptr - ptr -> int
                SharedExp res2 = constrainSub(ptrVal, ptrVal);

                if (!restrictTo) {
                    res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), intVal->clone()));
                }

                if (res) {
                    res = Binary::get(opOr, res, res2);
                }
                else {
                    res = res2;
                }
            }

            if (!restrictTo || restrictTo->isPointer()) {
                // ptr - int -> ptr
                SharedExp res2 = constrainSub(ptrVal, intVal);

                if (!restrictTo) {
                    res2 = Binary::get(opAnd, res2, Binary::get(opEquals, result->clone(), ptrVal->clone()));
                }

                if (res) {
                    res = Binary::get(opOr, res, res2);
                }
                else {
                    res = res2;
                }
            }

            if (res) {
                return res->simplify();
            }

            return Terminal::get(opFalse);
        }

        case opSize:
        {
            // This used to be considered obsolete, but now, it is used to carry the size of memOf's from the decoder to
            // here
            assert(subExp1->isIntConst());
            int sz = std::static_pointer_cast<const Const>(subExp1)->getInt();

            if (restrictTo) {
                int rsz = restrictTo->getSize();

                if (rsz == 0) {
                    // This is now restricted to the current restrictTo, but
                    // with a known size
                    SharedType it = restrictTo->clone();
                    it->setSize(sz);
                    return Binary::get(opEquals, Unary::get(opTypeOf, subExp2), TypeVal::get(it));
                }

                return Terminal::get((rsz == sz) ? opTrue : opFalse);
            }

            // We constrain the size but not the basic type
            return Binary::get(opEquals, result->clone(), TypeVal::get(SizeType::get(sz)));
        }

        default:
            break;
    }

    return Terminal::get(opTrue);
}


bool Binary::accept(ExpVisitor *v)
{
    assert(subExp1 && subExp2);

    bool override, ret = v->visit(shared_from_base<Binary>(), override);

    if (override) {
        return ret;
    }

    if (ret) {
        ret = subExp1->accept(v);
    }

    if (ret) {
        ret = subExp2->accept(v);
    }

    return ret;
}


SharedConstExp Binary::getSubExp2() const
{
    assert(subExp1 && subExp2);
    return subExp2;
}


SharedExp Binary::simplifyConstraint()
{
    assert(subExp1 && subExp2);

    subExp1 = subExp1->simplifyConstraint();
    subExp2 = subExp2->simplifyConstraint();

    switch (m_oper)
    {
        case opEquals:

            if (subExp1->isTypeVal() && subExp2->isTypeVal()) {
                // FIXME: ADHOC TA assumed
                SharedType t1 = subExp1->access<TypeVal>()->getType();
                SharedType t2 = subExp2->access<TypeVal>()->getType();

                if (!t1->isPointerToAlpha() && !t2->isPointerToAlpha()) {
                    if (*t1 == *t2) {
                        return Terminal::get(opTrue);
                    }
                    else {
                        return Terminal::get(opFalse);
                    }
                }
            }

            break;

        case opOr:
        case opAnd:
        case opNot:
            return simplify();

        default:
            break;
    }

    return shared_from_this();
}


SharedExp Binary::accept(ExpModifier *v)
{
    assert(subExp1 && subExp2);

    bool      recur;
    SharedExp ret = v->preVisit(shared_from_base<Binary>(), recur);

    if (recur) {
        subExp1 = subExp1->accept(v);
    }

    if (recur) {
        subExp2 = subExp2->accept(v);
    }

    auto bret = std::dynamic_pointer_cast<Binary>(ret);

    if (bret) {
        return v->postVisit(bret);
    }

    auto uret = std::dynamic_pointer_cast<Unary>(ret);

    if (uret) {
        return v->postVisit(uret);
    }

    Q_ASSERT(false);
    return nullptr;
}


void Binary::printx(int ind) const
{
    assert(subExp1 && subExp2);
    Util::alignStream(LOG_STREAM(), ind) << operToString(m_oper) << "\n";
    LOG_STREAM().flush();
    child(subExp1, ind);
    child(subExp2, ind);
}
