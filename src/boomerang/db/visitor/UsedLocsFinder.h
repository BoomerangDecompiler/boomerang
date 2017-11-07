#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License


#include "boomerang/db/visitor/ExpVisitor.h"

class LocationSet;


class UsedLocsFinder : public ExpVisitor
{
public:
    UsedLocsFinder(LocationSet& _used, bool _memOnly);
    ~UsedLocsFinder() override = default;

    LocationSet *getLocSet() { return m_used; }
    void setMemOnly(bool b)
    {
        m_memOnly = b;
    }

    bool isMemOnly() { return m_memOnly; }

    bool visit(const std::shared_ptr<RefExp>& e, bool& override) override;

    // Add used locations finder
    bool visit(const std::shared_ptr<Location>& e, bool& override) override;
    bool visit(const std::shared_ptr<Terminal>& e) override;

private:
    LocationSet *m_used; // Set of Exps
    bool m_memOnly;      // If true, only look inside m[...]
};
