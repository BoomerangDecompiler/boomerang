/*==============================================================================
 * FILE:       LoaderTest.cc
 * OVERVIEW:   Provides the implementation for the LoaderTest class, which
 *              tests the BinaryFile and derived classes
 *============================================================================*/
/*
 * $Revision$
 *
 * 05 Apr 02 - Mike: Created
 */

// Check that BOOMDIR is set
#ifndef BOOMDIR
#error BOOMDIR must be set!
#endif

#define HELLO_SPARC     BOOMDIR "/test/sparc/hello"
#define HELLO_PENTIUM   BOOMDIR "/test/pentium/hello"
#define HELLO_HPPA      BOOMDIR "/test/hppa/hello"
#define STARTER_PALM    BOOMDIR "/test/mc68328/Starter.prc"

#include "LoaderTest.h"
#include "util.h"           // For str()

/*==============================================================================
 * FUNCTION:        LoaderTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<LoaderTest> ("testExp", \
    &LoaderTest::name, *this))

void LoaderTest::registerTests(CppUnit::TestSuite* suite) {
    MYTEST(testSparcLoad);
    MYTEST(testPentiumLoad);
    MYTEST(testHppaLoad);
    MYTEST(testPalmLoad);
}

int LoaderTest::countTestCases () const
{ return 3; }   // ? What's this for?

/*==============================================================================
 * FUNCTION:        LoaderTest::setUp
 * OVERVIEW:        Set up anything needed before all tests
 * NOTE:            Called before any tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void LoaderTest::setUp () {
}

/*==============================================================================
 * FUNCTION:        LoaderTest::tearDown
 * OVERVIEW:        Delete objects created in setUp
 * NOTE:            Called after all tests
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void LoaderTest::tearDown () {
}

/*==============================================================================
 * FUNCTION:        LoaderTest::testSparcLoad
 * OVERVIEW:        Test loading the sparc hello world program
 *============================================================================*/
void LoaderTest::testSparcLoad () {
    std::ostringstream ost;

    // Load SPARC hello world
    BinaryFile* pBF = BinaryFile::Load(HELLO_SPARC);
    if (pBF == NULL) exit(1);
    int i, n;
    SectionInfo* si;
    n = pBF->GetNumSections();
    ost << "Number of sections = " << std::dec << n << "\r\n";
    for (int i=0; i < n; i++) {
        si = pBF->GetSectionInfo(i);
        ost << si->pSectionName << "\t";
    }
    pBF->UnLoad();
    // Note: the string below needs to have embedded tabs. Edit with caution!
    std::string expected("Number of sections = 28\r\n\t"
        ".interp	.hash	.dynsym	.dynstr	.SUNW_version	.rela.bss	"
        ".rela.plt	.text	.init	.fini	.rodata	.got	"
        ".plt	.dynamic	.data	.ctors	.dtors	.eh_frame	"
        ".bss	.symtab	.strtab	.comment	.stab.index	.stab	"
        ".shstrtab	.stab.indexstr	.stabstr	");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(str(ost)));
}

/*==============================================================================
 * FUNCTION:        LoaderTest::testPentiumLoad
 * OVERVIEW:        Test loading the pentium (Solaris) hello world program
 *============================================================================*/
void LoaderTest::testPentiumLoad () {
    std::ostringstream ost;

    // Load Pentium hello world
    BinaryFile* pBF = BinaryFile::Load(HELLO_PENTIUM);
    if (pBF == NULL) exit(1);
    int i, n;
    SectionInfo* si;
    n = pBF->GetNumSections();
    ost << "Number of sections = " << std::dec << n << "\r\n";
    for (int i=0; i < n; i++) {
        si = pBF->GetSectionInfo(i);
        ost << si->pSectionName << "\t";
    }
    pBF->UnLoad();
    // Note: the string below needs to have embedded tabs. Edit with caution!
    // (And slightly different string to the sparc test, e.g. rel vs rela)
    std::string expected("Number of sections = 28\r\n\t"
        ".interp	.hash	.dynsym	.dynstr	.SUNW_version	.rel.bss	"
        ".rel.plt	.plt	.text	.init	.fini	.rodata	.got	"
        ".dynamic	.data	.ctors	.dtors	.eh_frame	"
        ".bss	.symtab	.strtab	.comment	.stab.index	"
        ".debug	.debug_pubnames	.shstrtab	.stab.indexstr	");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(str(ost)));
}

/*==============================================================================
 * FUNCTION:        LoaderTest::testHppaLoad
 * OVERVIEW:        Test loading the sparc hello world program
 *============================================================================*/
void LoaderTest::testHppaLoad () {
    std::ostringstream ost;

    // Load HPPA hello world
    BinaryFile* pBF = BinaryFile::Load(HELLO_HPPA);
    if (pBF == NULL) exit(1);
    int i, n;
    SectionInfo* si;
    n = pBF->GetNumSections();
    ost << "Number of sections = " << std::dec << n << "\r\n";
    for (int i=0; i < n; i++) {
        si = pBF->GetSectionInfo(i);
        ost << si->pSectionName << "\t";
    }
    pBF->UnLoad();
    // Note: the string below needs to have embedded tabs. Edit with caution!
    std::string expected("Number of sections = 4\r\n"
        "$HEADER$	$TEXT$	$DATA$	$BSS$	");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(str(ost)));
}

/*==============================================================================
 * FUNCTION:        LoaderTest::testPalmLoad
 * OVERVIEW:        Test loading the Palm 68328 Starter.prc program
 *============================================================================*/
void LoaderTest::testPalmLoad () {
    std::ostringstream ost;

    // Load Palm Starter.prc
    BinaryFile* pBF = BinaryFile::Load(STARTER_PALM);
    if (pBF == NULL) exit(1);
    int i, n;
    SectionInfo* si;
    n = pBF->GetNumSections();
    ost << "Number of sections = " << std::dec << n << "\r\n";
    for (int i=0; i < n; i++) {
        si = pBF->GetSectionInfo(i);
        ost << si->pSectionName << "\t";
    }
    pBF->UnLoad();
    // Note: the string below needs to have embedded tabs. Edit with caution!
    std::string expected("Number of sections = 8\r\n"
        "code1	MBAR1000	tFRM1000	Talt1001	"
        "data0	code0	tAIN1000	tver1000	");
    CPPUNIT_ASSERT_EQUAL(expected, std::string(str(ost)));
}

