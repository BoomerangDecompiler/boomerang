#ifndef CPPUNIT_TESTSUITE_H    // -*- C++ -*-
#define CPPUNIT_TESTSUITE_H

#include <cppunit/Portability.h>
#include <cppunit/Test.h>
#include <vector>
#include <string>

namespace CppUnit {

class TestResult;

/*! \brief A Composite of Tests.
 *
 * It runs a collection of test cases. Here is an example.
 * \code
 * CppUnit::TestSuite *suite= new CppUnit::TestSuite();
 * suite->addTest(new CppUnit::TestCaller<MathTest> (
 *                  "testAdd", testAdd));
 * suite->addTest(new CppUnit::TestCaller<MathTest> (
 *                  "testDivideByZero", testDivideByZero));
 * \endcode
 * Note that TestSuites assume lifetime
 * control for any tests added to them.
 *
 * TestSuites do not register themselves in the TestRegistry.
 * \see Test 
 * \see TestCaller
 */


class TestSuite : public Test
{
public:
    TestSuite       (std::string name = "");
    ~TestSuite      ();

    void                run             (TestResult *result);
    int                 countTestCases  () const;
    std::string         getName         () const;
    std::string         toString        () const;

    void                addTest         (Test *test);
    const std::vector<Test *> & getTests() const;

    virtual void        deleteContents  ();

private:
    TestSuite (const TestSuite& other);
    TestSuite& operator= (const TestSuite& other); 

private:
    std::vector<Test *> m_tests;
    const std::string   m_name;
};


} // namespace CppUnit

#endif // CPPUNIT_TESTSUITE_H
