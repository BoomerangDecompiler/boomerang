#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

#include "prog.h"

class ParserTest : public CppUnit::TestCase {
  protected:

  public:
	ParserTest(std::string name) : CppUnit::TestCase (name)
	{}

	virtual void registerTests(CppUnit::TestSuite* suite);

	int countTestCases () const;

	void setUp ();
	void tearDown ();

	void testRead ();
	void testExp ();
};

