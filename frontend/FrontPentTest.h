#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include "BinaryFile.h"
#include "decoder.h"		// Actually use this class in the .cpp file

class FrontEnd;
class PentiumFrontEnd;

class FrontPentTest : public CppUnit::TestCase {
  protected:

  public:
	FrontPentTest(std::string name) : CppUnit::TestCase (name)
	{}

	virtual void registerTests(CppUnit::TestSuite* suite);

	int countTestCases () const;

	void setUp ();
	void tearDown ();

	void test1 ();
	void test2 ();
	void test3 ();
	void testBranch();
};

