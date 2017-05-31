/***************************************************************************/ /**
 * \file       ParserTest.cc
 * OVERVIEW:   Provides the implementation for the ParserTest class, which
 *                tests the sslparser.y etc
 ******************************************************************************/
#include "ParserTest.h"
#include "sslparser.h"
#include "boom_base/log.h"
#include "boom_base/log.h"

#include <QtCore/QDir>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QDebug>


#define SPARC_SSL    Boomerang::get()->getProgPath() + "frontend/machine/sparc/sparc.ssl"
static bool    logset = false;
static QString TEST_BASE;
static QDir    baseDir;

void ParserTest::initTestCase()
{
	if (!logset) {
		TEST_BASE = QProcessEnvironment::systemEnvironment().value("BOOMERANG_TEST_BASE", "");
		baseDir   = QDir(TEST_BASE);

		if (TEST_BASE.isEmpty()) {
			qWarning() << "BOOMERANG_TEST_BASE environment variable not set, will assume '..', many test may fail";
			TEST_BASE = "..";
			baseDir   = QDir("..");
		}

		logset = true;
		Boomerang::get()->setProgPath(TEST_BASE);
		Boomerang::get()->setPluginPath(TEST_BASE + "/out");
		Boomerang::get()->setLogger(new NullLogger());
	}
}


/***************************************************************************/ /**
 * \fn        ParserTest::testRead
 * OVERVIEW:        Test reading the SSL file
 ******************************************************************************/
void ParserTest::testRead()
{
	RTLInstDict d;

	QVERIFY(d.readSSLFile(SPARC_SSL));
}


/***************************************************************************/ /**
 * \fn        ParserTest::testExp
 * OVERVIEW:        Test parsing an expression
 ******************************************************************************/
void ParserTest::testExp()
{
	QString     s("*i32* r0 := 5 + 6");
	Instruction *a = SSLParser::parseExp(qPrintable(s));

	QVERIFY(a);
	QString     res;
	QTextStream ost(&res);
	a->print(ost);
	QCOMPARE(res, "   0 " + s);
	QString s2 = "*i32* r[0] := 5 + 6";
	a = SSLParser::parseExp(qPrintable(s2));
	QVERIFY(a);
	res.clear();
	a->print(ost);
	// Still should print to string s, not s2
	QCOMPARE(res, "   0 " + s);
}


QTEST_MAIN(ParserTest)
