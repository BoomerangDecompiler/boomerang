#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

#include "prog.h"

class ProgTest : public CppUnit::TestCase {
  protected:
	Prog*  m_prog;

  public:
	ProgTest(std::string name) : CppUnit::TestCase (name)
	{}

	virtual void registerTests(CppUnit::TestSuite* suite);

	int countTestCases () const;

	void setUp ();
	void tearDown ();

	void testName ();
};

