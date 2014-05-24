#pragma once

#include "codeblock.pb.h"

class TestLoader
{
public:
    TestLoader();
    bool readFromString(const char *data, CodeBlock &tgt);
};
