#include <QtTest/QTest>

class ParserTest : public QObject {
    Q_OBJECT
  public:
    virtual void SetUp() {}
    virtual void TearDown() {}
  private slots:
    void testRead();
    void testExp();
};
