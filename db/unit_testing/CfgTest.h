#include "cfg.h"
#include <QtTest/QTest>
class CfgTest : public QObject {
    Q_OBJECT
  protected:
    Cfg *m_prog;

  public:
    CfgTest();
    /***************************************************************************/ /**
      * FUNCTION:        RtlTest::setUp
      * OVERVIEW:        Set up some expressions for use with all the tests
      * NOTE:            Called before any tests
      * PARAMETERS:        <none>
      *
      *============================================================================*/
    virtual void SetUp();
    /***************************************************************************/ /**
      * FUNCTION:        RtlTest::tearDown
      * OVERVIEW:        Delete expressions created in setUp
      * NOTE:            Called after all tests
      * PARAMETERS:        <none>
      *
      *============================================================================*/
    virtual void TearDown();
  private slots:
    void initTestCase();
    void testDominators();
    void testSemiDominators();
    void testPlacePhi();
    void testPlacePhi2();
    void testRenameVars();
};
