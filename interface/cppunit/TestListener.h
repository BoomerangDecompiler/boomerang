#ifndef CPPUNIT_TESTLISTENER_H    // -*- C++ -*-
#define CPPUNIT_TESTLISTENER_H


namespace CppUnit {

class Exception;
class Test;


/*! \brief A listener for test progress.
 *
 * \see TestResult
 */
class TestListener
{
public:
    virtual ~TestListener() {}
    
    virtual void startTest( Test *test ) {}
    virtual void addError( Test *test, Exception *e ) {}
    virtual void addFailure( Test *test, Exception *e ) {}
    virtual void endTest( Test *test ) {}
};


} // namespace CppUnit

#endif // CPPUNIT_TESTLISTENER_H


