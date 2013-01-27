// address: 0x804837c
int main(int argc, int argv, int envp) {
    double st; 		// r32
    double st_1; 		// r32{29}
    double st_10; 		// r32{486}
    double st_11; 		// r32{537}
    double st_12; 		// r32{587}
    double st_2; 		// r32{80}
    double st_3; 		// r32{131}
    double st_4; 		// r32{182}
    double st_5; 		// r32{233}
    double st_6; 		// r32{283}
    double st_7; 		// r32{333}
    double st_8; 		// r32{384}
    double st_9; 		// r32{435}

    proc1();
    st_1 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st - st_1;
    proc1();
    st_2 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st - st_2;
    proc1();
    st_3 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st_3 - st;
    proc1();
    st_4 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st_4 - st;
    proc1();
    st_5 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st - st_5;
    proc1();
    st_6 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st_6 - st;
    proc1();
    st_7 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st / st_7;
    proc1();
    st_8 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st / st_8;
    proc1();
    st_9 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st_9 / st;
    proc1();
    st_10 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st_10 / st;
    proc1();
    st_11 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st / st_11;
    proc1();
    st_12 = (float)*0x80486c0;
    st = (float)*0x80486c4;
    res1 = st_12 / st;
    proc1();
    st = (float)*0x80486c0;
    st = st - (float)*0x80486c4;
    res1 = st;
    proc1();
    st = (float)*0x80486c0;
    st = (float)*0x80486c4 - st;
    res1 = st;
    proc1();
}

