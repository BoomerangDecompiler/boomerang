#pragma once

class CodeBlock;

#include <QtTest/QTest>

class TestLoader : public QObject
{
	Q_OBJECT

public:
	TestLoader();
	bool readFromString(const char *data, CodeBlock &tgt);
};
