#ifndef CPPUNIT_TESTRESULT_H
#define CPPUNIT_TESTRESULT_H

#include <vector>
#include <cppunit/TestFailure.h>

namespace CppUnit {

  class Exception;
  class Test;
  class TestListener;


  /**
   * A TestResult collects the results of executing a test case. It is an 
   * instance of the Collecting Parameter pattern.
   *
   * The test framework distinguishes between failures and errors.
   * A failure is anticipated and checked for with assertions. Errors are
   * unanticipated problems signified by exceptions that are not generated
   * by the framework.
   *
   * TestResult supplies a template method 'setSynchronizationObject ()'
   * so that subclasses can provide mutual exclusion in the face of multiple
   * threads.  This can be useful when tests execute in one thread and
   * they fill a subclass of TestResult which effects change in another 
   * thread.  To have mutual exclusion, override setSynchronizationObject ()
   * and make sure that you create an instance of ExclusiveZone at the 
   * beginning of each method.
   *
   * \see Test
   */

  class TestResult
  {

    public:
      TestResult  ();
      virtual                             ~TestResult ();

      virtual void                        addError       (Test *test, Exception *e);
      virtual void                        addFailure     (Test *test, Exception *e);
      virtual void                        startTest      (Test *test);
      virtual void                        endTest        (Test *test);
      virtual int                         runTests       ();
      virtual int                         testErrors     ();
      virtual int                         testFailures   ();
      virtual bool                        wasSuccessful  ();
      virtual bool                        shouldStop     ();
      virtual void                        stop           ();

      virtual std::vector<TestFailure *>& errors         ();
      virtual std::vector<TestFailure *>& failures       ();

      virtual void                        addListener    ( TestListener *listener );
      virtual void                        removeListener ( TestListener *listener );


      class SynchronizationObject
      {
        public:
          SynchronizationObject  () {}
          virtual                 ~SynchronizationObject () {}

          virtual void            lock                   () {}
          virtual void            unlock                 () {}
      };

      class ExclusiveZone
      {
          SynchronizationObject   *m_syncObject;

        public:
          ExclusiveZone (SynchronizationObject *syncObject) 
            : m_syncObject (syncObject) 
          { m_syncObject->lock (); }

          ~ExclusiveZone () 
          { m_syncObject->unlock (); }
      };

    protected:
      virtual void                setSynchronizationObject (SynchronizationObject *syncObject);

      std::vector<TestFailure *>  m_errors;
      std::vector<TestFailure *>  m_failures;
      std::vector<TestListener *> m_listeners;
      int                         m_runTests;
      bool                        m_stop;
      SynchronizationObject       *m_syncObject;
    private: 
      TestResult (const TestResult& other); 
      TestResult& operator= (const TestResult& other); 

  };

} // namespace CppUnit

#endif // CPPUNIT_TESTRESULT_H


