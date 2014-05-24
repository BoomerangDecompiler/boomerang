#include <iostream>        // For std::cerr
#include "gtest/gtest.h"
#include "log.h"
class ErrLogger : public Log {
public:
    virtual Log &operator<<(const std::string& s) {
     std::cerr << s;
        return *this;
    }
    virtual ~ErrLogger() {}
};
class DfaTest : public ::testing::Test {
public:
    DfaTest();
    virtual void SetUp();
    virtual void TearDown();
};
