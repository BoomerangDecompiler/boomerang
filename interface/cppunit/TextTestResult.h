#ifndef CPPUNIT_TEXTTESTRESULT_H
#define CPPUNIT_TEXTTESTRESULT_H

#include <iostream>
#include <cppunit/TestResult.h>

namespace CppUnit {

  class Exception;
  class Test;

  class TextTestResult : public TestResult 
  {
    public:
      virtual void        addError      (Test *test, Exception *e);
      virtual void        addFailure    (Test *test, Exception *e);
      virtual void        startTest     (Test *test);
      virtual void        print         (std::ostream& stream);
      virtual void        printErrors   (std::ostream& stream);
      virtual void        printFailures (std::ostream& stream);
      virtual void        printHeader   (std::ostream& stream);
      
  };

  /** insertion operator for easy output */
  std::ostream& operator<< (std::ostream& stream, TextTestResult& result);

} // namespace CppUnit

#endif // CPPUNIT_TEXTTESTRESULT_H


