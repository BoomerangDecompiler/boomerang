/void test()/,/int local0/ c \
void test(int i) { \
    int local0 = i;
/\*(int\*)/ d
/test()/ d
/return 0/ i \
    test(-5); \
    test(-2); \
    test(0); \
    test(argc); \
    test(5);
s/\&global/global/
