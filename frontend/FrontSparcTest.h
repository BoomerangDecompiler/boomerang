#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include "BinaryFile.h"

class FrontEnd;
class SparcFrontEnd;
class NJMCDecoder;
class Prog;


class FrontSparcTest : public CppUnit::TestCase {
  protected:

  public:
	FrontSparcTest(std::string name) : CppUnit::TestCase (name)
	{}

	virtual void registerTests(CppUnit::TestSuite* suite);

	int countTestCases () const;

	void setUp ();
	void tearDown ();

	void test1 ();
	void test2 ();
	void test3 ();
	void testBranch();
	void testDelaySlot();
};

