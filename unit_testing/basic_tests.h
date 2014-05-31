#ifndef BASIC_TESTS_H
#define BASIC_TESTS_H
#include <QtTest/QtTest>
class CodeBlockLoader : public QObject {
    Q_OBJECT
  private slots:
    void load_basic_data();
};

#endif // BASIC_TESTS_H
