/***************************************************************************//**
 * \file    FrontendTest.cpp
 * Provides the implementation for the FrontendTest class, which
 *          tests the FrontEnd and related classes
 *============================================================================*/
/*
 * $Revision: 1.5 $
 *
 * 05 Apr 02 - Mike: Created
 */

#include "FrontendTest.h"
#include "prog.h"

#define HELLO_SPARC     "test/sparc/hello"
#define HELLO_PENTIUM   "test/pentium/hello"
#define HELLO_HPPA      "test/hppa/hello"
#define STARTER_PALM    "test/mc68328/Starter.prc"

/***************************************************************************//**
 * \brief Set up anything needed before all tests
 * \note Called before any tests
 *============================================================================*/
void FrontendTest::SetUp () {
}

/***************************************************************************//**
 * \brief   Delete objects created in setUp
 * \note    Called after all tests
 *============================================================================*/
void FrontendTest::TearDown () {
}

/***************************************************************************//**
 * \brief Test loading the sparc hello world program
 *============================================================================*/
TEST_F(FrontendTest,test1) {
}

