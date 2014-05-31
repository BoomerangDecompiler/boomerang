#include "log.h"

#include <iostream> // For std::cerr
#include <QtTest/QTest>

class ErrLogger : public Log {
  public:
    virtual Log &operator<<(const QString &s) {
        std::cerr << s.toStdString();
        return *this;
    }
    virtual ~ErrLogger() {}
};
class DfaTest : public QObject {
    Q_OBJECT
  public:
    DfaTest();
    virtual void SetUp();
    virtual void TearDown();
  private slots:
    void testMeetInt();
    void testMeetSize();
    void testMeetPointer();
    void testMeetUnion();
};
