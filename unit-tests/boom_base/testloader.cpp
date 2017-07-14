
#include "testloader.h"

TestLoader::TestLoader()
{}

bool TestLoader::readFromString(const char *data, CodeBlock &tgt)
{
    Q_UNUSED(data);
    Q_UNUSED(tgt);

    return false;
}

QTEST_MAIN(TestLoader)
