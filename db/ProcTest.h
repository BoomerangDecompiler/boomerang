/*==============================================================================
 * FILE:       ProcTest.h
 * OVERVIEW:   Provides the interface for the ProcTest class, which
 *              tests the Proc class
 *============================================================================*/
/*
 * $Revision$
 *
 * 23 Apr 02 - Mike: Created
 */

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

#include "proc.h"
#include "prog.h"

class ProcTest : public CppUnit::TestCase {
  protected:
    Prog*  m_prog;
    Proc*  m_proc;
    

  public:
    ProcTest(std::string name) : CppUnit::TestCase (name)
    {}

    virtual void registerTests(CppUnit::TestSuite* suite);

    int countTestCases () const;

    void setUp ();
    void tearDown ();

    void testName ();
};

