#include "gtest/gtest.h"
#include "testloader.h"
#include "codeblock.pb.h"

const char *test_data = {""
"machine: {"
"        arch: 'x86'"
"        cpu: '80686'"
"        os: 'unknown'"
"        hw: 'unknown'"
"}"
"loader: {"
"        load_at: 0"
"}"
"data: '111\01'"
};

class CodeBlockLoaderFixture : public ::testing::Test {
public:
    TestLoader loader;
    virtual void SetUp() {
    }
    virtual void TearDown() {
    }
};
TEST_F(CodeBlockLoaderFixture, load_basic_data) {
    CodeBlock b;
    ASSERT_TRUE(loader.readFromString(test_data,b));
    ASSERT_TRUE(b.has_machine());
    ASSERT_TRUE(b.has_loader());
    ASSERT_TRUE(b.has_data());
}
