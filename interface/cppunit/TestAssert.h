#ifndef CPPUNIT_TESTASSERT_H
#define CPPUNIT_TESTASSERT_H

#include <cppunit/Portability.h>
#include <cppunit/Exception.h>
#include <string>


namespace CppUnit {

  template <class T>
  struct assertion_traits 
  {  
      static bool equal( const T& x, const T& y )
      {
          return x == y;
      }

      static std::string toString( const T& x )
      {
          OStringStream ost;
          ost << x;
          return ost.str();
      }
  };


  namespace TestAssert
  {
    void assertImplementation( bool         condition, 
                               std::string  conditionExpression = "",
                               long lineNumber = Exception::UNKNOWNLINENUMBER,
                               std::string  fileName = Exception::UNKNOWNFILENAME );

    void assertNotEqualImplementation( std::string expected,
                                       std::string actual,
                                       long lineNumber = Exception::UNKNOWNLINENUMBER,
                                       std::string fileName = Exception::UNKNOWNFILENAME );
      

    template <class T>
    void assertEquals( const T& expected,
                       const T& actual,
                       long lineNumber = Exception::UNKNOWNLINENUMBER,
                       std::string fileName = Exception::UNKNOWNFILENAME )
    {
      if ( !assertion_traits<T>::equal(expected,actual) ) // lazy toString conversion...
      {
        assertNotEqualImplementation( assertion_traits<T>::toString(expected),
                                      assertion_traits<T>::toString(actual),
                                      lineNumber, 
                                      fileName );
      }
    }

    void assertEquals( double expected, 
                       double actual, 
                       double delta, 
                       long lineNumber = Exception::UNKNOWNLINENUMBER,
                       std::string fileName = Exception::UNKNOWNFILENAME);
  }


/** A set of macros which allow us to get the line number
 * and file name at the point of an error.
 * Just goes to show that preprocessors do have some
 * redeeming qualities.
 */
#if CPPUNIT_HAVE_CPP_SOURCE_ANNOTATION

#  define CPPUNIT_ASSERT(condition)\
    (CppUnit::TestAssert::assertImplementation ((condition),(#condition),\
        __LINE__, __FILE__))

#else

#  define CPPUNIT_ASSERT(condition)\
    (CppUnit::TestAssert::assertImplementation ((condition),"",\
        __LINE__, __FILE__))

#endif

/** Assertion with a user specified message.
 * \param message Message reported in diagnostic if \a condition evaluates
 *                to \c false.
 * \param condition If this condition evaluates to \c false then the
 *                  test failed.
 */
#define CPPUNIT_ASSERT_MESSAGE(message,condition)\
  (CppUnit::TestAssert::assertImplementation( condition, \
                                              message, \
                                              __LINE__, \
                                              __FILE__ ) )

/// Generalized macro for primitive value comparisons
/** Equality and string representation can be defined with
 * an appropriate assertion_traits class.
 * A diagnostic is printed if actual and expected values disagree.
 */
#define CPPUNIT_ASSERT_EQUAL(expected,actual)\
  (CppUnit::TestAssert::assertEquals ((expected),\
    (actual),__LINE__,__FILE__))

/// Macro for primitive value comparisons
#define CPPUNIT_ASSERT_DOUBLES_EQUAL(expected,actual,delta)\
  (CppUnit::TestAssert::assertEquals ((expected),\
    (actual),(delta),__LINE__,__FILE__))


// Backwards compatibility

#if CPPUNIT_ENABLE_NAKED_ASSERT

#undef assert
#define assert(c)                 CPPUNIT_ASSERT(c)
#define assertEqual(e,a)          CPPUNIT_ASSERT_EQUAL(e,a)
#define assertDoublesEqual(e,a,d) CPPUNIT_ASSERT_DOUBLES_EQUAL(e,a,d)
#define assertLongsEqual(e,a)     CPPUNIT_ASSERT_EQUAL(e,a)

#endif


} // namespace CppUnit

#endif  // CPPUNIT_TESTASSERT_H
