/*==============================================================================
 * FILE:       LoaderTest.cc
 * OVERVIEW:   Provides the implementation for the LoaderTest class, which
 *              tests the BinaryFile and derived classes
 *============================================================================*/
/*
 * $Revision$
 *
 * 05 Apr 02 - Mike: Created
 * 14 Jun 02 - Mike: Added windows test for calc.exe
 * 20 Jun 02 - Mike: Added test for microX86Dis
 * 08 Jul 02 - Mike: Test more exe file formats
 */

// Check that BOOMDIR is set
#ifndef BOOMDIR
#error BOOMDIR must be set!
#endif

#define HELLO_SPARC     BOOMDIR "/test/sparc/hello"
#define HELLO_PENTIUM   BOOMDIR "/test/pentium/hello"
#define HELLO_HPPA      BOOMDIR "/test/hppa/hello"
#define STARTER_PALM    BOOMDIR "/test/mc68328/Starter.prc"
#define CALC_WINDOWS    BOOMDIR "/test/windows/calc.exe"
#define CALC_WINXP      BOOMDIR "/test/windows/calcXP.exe"
#define CALC_WIN2000    BOOMDIR "/test/windows/calc2000.exe"
#define LPQ_WINDOWS     BOOMDIR "/test/windows/lpq.exe"

#include "LoaderTest.h"
//#include "util.h"           // For str()
#include <iostream>         // For cout

/*==============================================================================
 * FUNCTION:        LoaderTest::registerTests
 * OVERVIEW:        Register the test functions in the given suite
 * PARAMETERS:      Pointer to the test suite
 * RETURNS:         <nothing>
 *============================================================================*/
#define MYTEST(name) \
suite->addTest(new CppUnit::TestCaller<LoaderTest> ("LoaderTest", \
    &LoaderTest::name, *this))

void LoaderTest::registerTests(CppUnit::TestSuite* suite) {
    MYTEST(testSparcLoad);
    MYTEST(testPentiumLoad);
    MYTEST(testHppaLoad);
    MYTEST(testPalmLoad);
    MYTEST(testWinLoad);

    MYTEST(testMicroDis);
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
    int n;
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
    CPPUNIT_ASSERT_EQUAL(expected, ost.str());
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
    int n;
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
    CPPUNIT_ASSERT_EQUAL(expected, ost.str());
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
    int n;
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
    CPPUNIT_ASSERT_EQUAL(expected, ost.str());
	delete pBF;
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
    int n;
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
    CPPUNIT_ASSERT_EQUAL(expected, ost.str());
	delete pBF;
}

/*==============================================================================
 * FUNCTION:        LoaderTest::testWinLoad
 * OVERVIEW:        Test loading the Windows calc.exe program
 *============================================================================*/
void LoaderTest::testWinLoad () {
    std::ostringstream ost;

    // Load Windows program calc.exe
    BinaryFile* pBF = BinaryFile::Load(CALC_WINDOWS);
    if (pBF == NULL) exit(1);
    int n;
    SectionInfo* si;
    n = pBF->GetNumSections();
    ost << "Number of sections = " << std::dec << n << "\r\n";
    for (int i=0; i < n; i++) {
        si = pBF->GetSectionInfo(i);
        ost << si->pSectionName << "\t";
    }

    // Note: the string below needs to have embedded tabs. Edit with caution!
    std::string expected("Number of sections = 5\r\n"
        ".text	.rdata	.data	.rsrc	.reloc	");
    std::string actual(ost.str());
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    ADDRESS addr = pBF->GetMainEntryPoint();
    CPPUNIT_ASSERT(addr != NO_ADDRESS);

    // Test symbol table (imports)
    char* s = pBF->SymbolByAddress(0x1292060U);
    if (s == 0)
        actual = "<not found>";
    else
        actual = std::string(s);
    expected = std::string("SetEvent");
    CPPUNIT_ASSERT_EQUAL(expected, actual);

    ADDRESS a = pBF->GetAddressByName("SetEvent");
    ADDRESS expectedAddr = 0x1292060;
    CPPUNIT_ASSERT_EQUAL(expectedAddr, a);
    pBF->UnLoad();

    // Test loading the "new style" exes, as found in winXP etc
    pBF = BinaryFile::Load(CALC_WINXP);
    if (pBF == NULL) exit(1);
    addr = pBF->GetMainEntryPoint();
    CPPUNIT_ASSERT(addr != NO_ADDRESS);
    pBF->UnLoad();

    // Test loading the calc.exe found in Windows 2000 (more NT based)
    pBF = BinaryFile::Load(CALC_WIN2000);
    if (pBF == NULL) exit(1);
    addr = pBF->GetMainEntryPoint();
    CPPUNIT_ASSERT(addr != NO_ADDRESS);
    pBF->UnLoad();

    // Test loading the lpq.exe program - console mode PE file
    pBF = BinaryFile::Load(LPQ_WINDOWS);
    if (pBF == NULL) exit(1);
    addr = pBF->GetMainEntryPoint();
    CPPUNIT_ASSERT(addr != NO_ADDRESS);
    pBF->UnLoad();
	delete pBF;
}

/*==============================================================================
 * FUNCTION:        LoaderTest::testMicroDis
 * OVERVIEW:        Test the micro disassembler
 *============================================================================*/
extern "C" {
    int microX86Dis(void* p);
}

// The below lengths were derived from a quick and dirty program (called
// quick.c) which used the output from a disassembly to find the lengths.
// Best way to test, but of course this array is very dependent on the
// exact booked in test program
static char lengths[] = {
2, 2, 2, 1, 5, 2, 2, 5, 5, 3, 5, 2, 2, 5, 5, 5, 3, 4, 6, 1,
3, 1, 1, 5, 5, 5, 3, 1, 5, 2, 5, 7, 1, 1, 1, 2, 1, 5, 1, 6,
2, 1, 1, 3, 6, 2, 2, 6, 3, 2, 6, 1, 5, 3, 1, 1, 1, 1, 1, 1,
2, 1, 5, 1, 6, 3, 1, 1, 1, 1, 1, 1, 2, 1, 5, 1, 6, 6, 1, 6,
1, 5, 3, 1, 1, 1, 2, 1, 5, 1, 6, 3, 1, 1, /* main */ 2, 3, 1, 5, 5, 3,
2, 2, 1, /* label */ 1, 2, 1, 2, 1, 1, 2, 2, 1, 1, 1, 3, 3, 1, 3, 2, 5, 2,
2, 2, 2, 2, 3, 2, 3, 2, 3, 3, 1, 1, 1, 1, 1, 1, 2, 3, 1, 1,
3, 7, 3, 1, 1, 1, 3, 1, 2, 5, 2, 3, 3, 2, 2, 2, 3, 2, 6, 2,
5, 2, 3, 3, 3, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 1, 1,
3, 3, 3, 3, 2, 2, 3, 1, 2, 3, 3, 4, 3, 3, 3, 2, 2, 2, 2, 3,
2, 3, 3, 4, 3, 1, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 2, 3,
2, 1, 1, 1, 4, 2, 4, 2, 1, 2, 2, 3, 4, 2, 2, 1, 1, 1, 1, 1,
2, 3, 1, 1, 1, 5, 1, 6, 3, 3, 2, 3, 2, 3, 3, 2, 3, 3, 2, 1,
1, 4, 2, 4, 2, 1, 1, 1, 3, 5, 3, 3, 3, 2, 3, 3, 3, 2, 3, 2,
2, 3, 4, 2, 3, 2, 3, 3, 2, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1,
1, 2, 3, 1, 1, 1, 5, 1, 6, 3, 3, 2, 2, 2, 7, 3, 2, 1, 1, 1,
2, 5, 3, 3, 3, 3, 2, 2, 1, 3, 3, 5, 3, 3, 3, 3, 3, 3, 1, 5,
2, 7, 2, 3, 3, 3, 3, 3, 2, 2, 2, 3, 2, 3, 3, 1, 1, 3, 3, 1,
3, 1, 1, 2, 5, 3, 3, 3, 2, 2, 3, 1, 3, 1, 3, 1, 1, 3, 3, 5,
3, 3, 3, 2, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 1, 1,
1, 5, 1, 6, 6, 2, 2, 1, 3, 2, 1, 5, 3, 3, 2, 2, 3, 2, 3, 2,
2, 2, 2, 2, 1, 3, 2, 1, 1, 1, 2, 3, 3, 2, 2, 3, 1, 3, 3, 2,
2, 3, 3, 3, 3, 2, 3, 2, 1, 1, 1, 3, 3, 3, 2, 3, 3, 2, 2, 3,
1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 1, 1, 1, 5, 1, 6, 3, 3, 5,
2, 3, 3, 3, 2, 3, 6, 3, 5, 2, 1, 2, 2, 2, 3, 6, 5, 1, 2, 2,
2, 4, 2, 2, 5, 1, 1, 1, 3, 2, 3, 2, 2, 2, 1, 5, 2, 2, 3, 4,
3, 2, 1, 3, 3, 6, 3, 5, 1, 2, 2, 2, 3, 3, 3, 3, 3, 2, 1, 1,
1, 3, 7, 3, 5, 1, 1, 5, 2, 3, 3, 1, 1, 5, 2, 3, 3, 3, 1, 2,
3, 3, 2, 3, 1, 1, 5, 2, 3, 2, 3, 1, 1, 1, 1, 1, 1, 1, 2, 3,
1, 1, 1, 5, 1, 6, 3, 3, 3, 3, 1, 3, 2, 6, 3, 2, 5, 4, 2, 5,
1, 1, 2, 2, 5, 3, 3, 1, 3, 5, 3, 3, 4, 3, 3, 3, 5, 3, 7, 3,
4, 5, 1, 1, 2, 2, 5, 3, 3, 3, 4, 5, 1, 5, 6, 2, 7, 2, 1, 1,
2, 2, 2, 2, 6, 2, 3, 2, 2, 4, 3, 2, 2, 2, 1, 2, 6, 2, 3, 2,
2, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
2, 2, 2, 3, 2, 2, 6, 2, 3, 3, 5, 1, 1, 3, 3, 2, 1, 3, 5, 1,
1, 1, 3, 3, 2, 3, 3, 5, 1, 2, 3, 2, 2, 3, 3, 5, 1, 1, 1, 1,
3, 1, 3, 5, 3, 3, 1, 3, 5, 3, 3, 4, 3, 3, 3, 5, 3, 7, 3, 4,
5, 1, 1, 1, 3, 1, 3, 5, 3, 3, 3, 5, 5, 1, 3, 1, 3, 5, 3, 3,
1, 3, 5, 3, 3, 3, 5, 3, 7, 3, 4, 5, 1, 3, 1, 3, 5, 3, 3, 1,
3, 5, 3, 3, 3, 4, 3, 3, 5, 1, 3, 1, 3, 5, 3, 3, 3, 4, 5, 1,
1, 3, 1, 3, 5, 3, 3, 3, 3, 5, 1, 1, 1, 2, 5, 2, 2, 3, 2, 1,
5, 2, 3, 3, 2, 3, 3, 2, 2, 2, 1, 5, 2, 1, 5, 2, 7, 1, 3, 3,
5, 3, 7, 4, 3, 3, 2, 5, 2, 2, 1, 1, 3, 1, 3, 5, 3, 3, 3, 3,
2, 1, 1, 5, 1, 1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 1, 2, 1, 5, 1,
6, 3, 3, 3, 7, 6, 7, 7, 6, 3, 6, 3, 1, 1, 1, 2, 1, 5, 1, 6,
3, 3, 3, 3, 7, 6, 7, 6, 3, 6, 3, 1, 1, 1, 2, 1, 5, 1, 6, 3,
6, 7, 2, 1, 1, 2, 3, 2, 3, 2, 3, 2, 3, 5, 2, 1, 3, 4, 2, 5,
1, 1, 3, 1, 1, 1, 1, 1, 1, 2, 6, 1, 1, 1, 5, 1, 6, 3, 5, 6,
3, 2, 6, 3, 6, 1, 6, 5, 2, 3, 2, 6, 2, 2, 6, 6, 1, 5, 3, 4,
3, 6, 3, 6, 3, 5, 2, 2, 2, 3, 2, 2, 6, 6, 6, 6, 1, 2, 6, 6,
1, 5, 2, 3, 2, 2, 6, 3, 3, 3, 2, 6, 1, 1, 5, 2, 6, 3, 6, 2,
3, 6, 3, 6, 2, 2, 6, 6, 3, 6, 2, 6, 3, 1, 6, 1, 1, 5, 2, 3,
2, 2, 3, 6, 1, 5, 2, 3, 6, 1, 1, 1, 1, /* label */ 1, 2, 1, 2, 1, 1, 5, 1,
6, 6, 3, 4, 2, 2, 2, 3, 3, 2, 3, 1, 1, 1, 1, 1, 1, 2, 1, 5,
1, 6, 3, 1, 1
};

void LoaderTest::testMicroDis () {
    std::ostringstream ost;

    // Load Pentium hello world
    BinaryFile* pBF = BinaryFile::Load(HELLO_PENTIUM);
    if (pBF == NULL) exit(1);
    int i;
    SectionInfo* si;
    i = pBF->GetSectionIndexByName(".text");
    si = pBF->GetSectionInfo(i);
    unsigned int n = si->uSectionSize;
    int totalSize = 0;
    void* p = (void*)si->uHostAddr;
    i = 0;
    while (totalSize < (int)n) {
//std::cout << std::hex << (ADDRESS)p + si->uNativeAddr - si->uHostAddr << "\t";
        int size = microX86Dis(p);
        if (size >= 0x40) {
            std::cout << "Not handled instruction at address " << std::hex <<
                (ADDRESS)p + si->uNativeAddr - si->uHostAddr << std::endl;
            CPPUNIT_ASSERT(size != 0x40);
            return;
        }
        int expected = lengths[i++];
        if (expected != size) {
            std::cout << "At address 0x" << std::hex <<
              (ADDRESS)p + si->uNativeAddr - si->uHostAddr << " expected " <<
              std::dec << expected << ", actual " << size << std::endl;
              CPPUNIT_ASSERT_EQUAL(expected, size);
        }
        p = (void*) ((char*)p + size);
        totalSize += size;
    }
    CPPUNIT_ASSERT_EQUAL((int)n, totalSize);


    // Now a special test:
    // 8048910:  0f be 00            movsbl (%eax),%eax
    // 8048913:  0f bf 00            movswl (%eax),%eax

    unsigned char movsbl[3] = {0x0f, 0xbe, 0x00};
    unsigned char movswl[3] = {0x0f, 0xbf, 0x00};
    int size = microX86Dis(movsbl);
    CPPUNIT_ASSERT_EQUAL(3, size);
    size = microX86Dis(movswl);
    CPPUNIT_ASSERT_EQUAL(3, size);
}
