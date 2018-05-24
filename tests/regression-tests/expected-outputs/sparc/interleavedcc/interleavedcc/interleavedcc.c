int main(int argc, char *argv[]);
void interleaved(int param1, __size32 param2, __size32 param3);

/** address: 0x00010acc */
int main(int argc, char *argv[])
{
    interleaved(1, 0x40000000, 0x40400000);
    printf(0x116c8);
    interleaved(-1, 0x40000000, 0x40400000);
    printf(0x116e8);
    interleaved(1, 0x40000000, 0x40000000);
    printf(0x11708);
    interleaved(-1, 0x40000000, 0x40000000);
    printf(0x11728);
    return 0;
}

/** address: 0x00010a80 */
void interleaved(int param1, __size32 param2, __size32 param3)
{
    if (param2 != param3) {
        if (param1 >= 0) {
        }
        else {
        }
    }
    else {
        if (param1 < 0) {
        }
    }
    return;
}

