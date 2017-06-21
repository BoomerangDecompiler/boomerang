#include "core/BinaryFileFactory.h"

#include <QtTest/QTest>

class LoaderTest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();

	/// test the loader using a "Hello World" program
	/// compiled with clang-4.0.0 (without debug info)
	void testElfLoadClang();

	/// test the loader using a "Hello World" program
	/// compiled with clang-4.0.0 (without debug info, static libc)
	void testElfLoadClangStatic();

	/// Test loading the sparc hello world program
	void testSparcLoad();

	/// Test loading the pentium (Solaris) hello world program
	void testPentiumLoad();

	/// Test loading the sparc hello world program
	void testHppaLoad();

	/// Test loading the Palm 68328 Starter.prc program
	void testPalmLoad();

	/// Test loading Windows programs
	void testWinLoad();

	/// Test the micro disassembler
	void testMicroDis1();
	void testMicroDis2();

	/// Test the ELF hash function.
	void testElfHash();
};
