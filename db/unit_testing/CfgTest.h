#include "cfg.h"
#include "gtest/gtest.h"
class CfgTest : public ::testing::Test {
protected:
    Cfg*  m_prog;
public:
    CfgTest();
    /***************************************************************************//**
     * FUNCTION:        RtlTest::setUp
     * OVERVIEW:        Set up some expressions for use with all the tests
     * NOTE:            Called before any tests
     * PARAMETERS:        <none>
     *
     *============================================================================*/
    virtual void SetUp();
    /***************************************************************************//**
     * FUNCTION:        RtlTest::tearDown
     * OVERVIEW:        Delete expressions created in setUp
     * NOTE:            Called after all tests
     * PARAMETERS:        <none>
     *
     *============================================================================*/
    virtual void TearDown();
};
