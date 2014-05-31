/*#include <google/protobuf/text_format.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>*/
#include "testloader.h"
//using namespace google::protobuf;

TestLoader::TestLoader() {}
bool TestLoader::readFromString(const char *data, CodeBlock &tgt) {
    /*io::ArrayInputStream iarr(data,strlen(data));
    return TextFormat::Parse(&iarr,&tgt);*/
    return false; // Disabled due to missing codeblock.proto
}
