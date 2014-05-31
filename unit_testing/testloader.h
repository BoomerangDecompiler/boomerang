#pragma once

class CodeBlock;

class TestLoader {
  public:
    TestLoader();
    bool readFromString(const char *data, CodeBlock &tgt);
};
