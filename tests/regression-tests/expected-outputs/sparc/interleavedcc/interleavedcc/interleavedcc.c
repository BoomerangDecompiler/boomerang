int main(int argc, char *argv[]);
__size32 interleaved(int param1, __size32 param2, __size32 param3);


/** address: 0x00010acc */
int main(int argc, char *argv[])
{
    int o0; 		// r8

    o0 = interleaved(1, 0x40000000, 0x40400000);
    printf("Call with  1, 2.0, 3.0: %d\n", o0);
    o0 = interleaved(-1, 0x40000000, 0x40400000);
    printf("Call with -1, 2.0, 3.0: %d\n", o0);
    o0 = interleaved(1, 0x40000000, 0x40000000);
    printf("Call with  1, 2.0, 2.0: %d\n", o0);
    o0 = interleaved(-1, 0x40000000, 0x40000000);
    printf("Call with -1, 2.0, 2.0: %d\n", o0);
    return 0;
}

/** address: 0x00010a80 */
__size32 interleaved(int param1, __size32 param2, __size32 param3)
{
    __size32 local0; 		// o0{13}
    __size32 o0; 		// r8
    __size32 o0_1; 		// r8{5}

    if (param2 != param3) {
        o0 = 1;
        local0 = o0;
        if (param1 >= 0) {
            o0 = 0;
            local0 = o0;
        }
    }
    else {
        o0_1 = 2;
        local0 = o0_1;
        if (param1 < 0) {
            o0 = 3;
            local0 = o0;
        }
    }
    o0 = local0;
    return o0;
}

