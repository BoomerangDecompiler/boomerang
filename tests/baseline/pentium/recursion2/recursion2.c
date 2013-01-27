int e_c = 3;
__size32 res;// 4 bytes
int d_e = 3;
int g_f = 3;
int f_g = 3;
int h_i = 3;
int k_e = 3;
int j_k = 3;
int l_b = 3;
int c_f = 3;
int c_d = 3;
int c_h = 3;
int c_j = 3;
int c_l = 3;
int b_c = 3;

void b(__size32 param1, __size32 param2);
void c(__size32 param1, __size32 param2);
__size32 d(__size32 param1, __size32 param2);
__size32 f(__size32 param1, __size32 param2);
void h();
void j();
void l();
__size32 e(__size32 param1, __size32 param2);
__size32 g(__size32 param1, __size32 param2);
void i();
void k();

// address: 0x804837c
int main(int argc, char *argv[], char *envp[]) {
    b(55, 99);
    proc1();
    proc1();
    return 0;
}

// address: 0x80483d6
void b(__size32 param1, __size32 param2) {
    b_c = b_c - 1;
    if (b_c >= 0) {
        c(param2, param1);
    }
    res += 2;
    return;
}

// address: 0x8048408
void c(__size32 param1, __size32 param2) {
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 edx_1; 		// r26{22}
    __size32 local0; 		// param1{102}
    __size32 local1; 		// param2{103}
    __size32 local2; 		// ecx{107}
    __size32 local3; 		// edx{108}

    c_d = c_d - 1;
    local0 = param1;
    local1 = param2;
    if (c_d >= 0) {
        ecx = d(param2, param1); /* Warning: also results in edx_1 */
        edx = ecx;
        ecx = edx_1;
        local0 = ecx;
        local1 = edx;
    }
    param1 = local0;
    param2 = local1;
    c_f = c_f - 1;
    local2 = param1;
    local3 = param2;
    if (c_f >= 0) {
        ecx = f(param1, param2); /* Warning: also results in edx */
        local2 = ecx;
        local3 = edx;
    }
    ecx = local2;
    edx = local3;
    c_h = c_h - 1;
    if (c_h >= 0) {
        h();
    }
    c_j = c_j - 1;
    if (c_j >= 0) {
        j();
    }
    c_l = c_l - 1;
    if (c_l >= 0) {
        l();
    }
    res += 3;
    return;
}

// address: 0x80484a6
__size32 d(__size32 param1, __size32 param2) {
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 edx_1; 		// r26{22}
    __size32 local0; 		// param1{38}
    __size32 local1; 		// param2{39}

    d_e = d_e - 1;
    local0 = param1;
    local1 = param2;
    if (d_e >= 0) {
        ecx = e(param2, param1); /* Warning: also results in edx_1 */
        edx = ecx;
        ecx = edx_1;
        local0 = ecx;
        local1 = edx;
    }
    param1 = local0;
    param2 = local1;
    res += 5;
    return param1; /* WARNING: Also returning: edx := param2 */
}

// address: 0x804850a
__size32 f(__size32 param1, __size32 param2) {
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 local0; 		// param1{32}
    __size32 local1; 		// param2{33}

    f_g = f_g - 1;
    local0 = param1;
    local1 = param2;
    if (f_g >= 0) {
        ecx = g(param1, param2); /* Warning: also results in edx */
        local0 = ecx;
        local1 = edx;
    }
    param1 = local0;
    param2 = local1;
    res += 11;
    return param1; /* WARNING: Also returning: edx := param2 */
}

// address: 0x8048566
void h() {
    h_i = h_i - 1;
    if (h_i >= 0) {
        i();
    }
    res += 17;
    return;
}

// address: 0x80485a6
void j() {
    j_k = j_k - 1;
    if (j_k >= 0) {
        k();
    }
    res += 23;
    return;
}

// address: 0x804860b
void l() {
    l_b = l_b - 1;
    if (l_b >= 0) {
        proc5();
    }
    res += 29;
    return;
}

// address: 0x80484d8
__size32 e(__size32 param1, __size32 param2) {
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 edx_1; 		// r26{22}
    __size32 local0; 		// param1{38}
    __size32 local1; 		// param2{39}

    e_c = e_c - 1;
    local0 = param1;
    local1 = param2;
    if (e_c >= 0) {
        proc2();
        edx = ecx;
        ecx = edx_1;
        local0 = ecx;
        local1 = edx;
    }
    param1 = local0;
    param2 = local1;
    res += 7;
    return param1; /* WARNING: Also returning: edx := param2 */
}

// address: 0x8048538
__size32 g(__size32 param1, __size32 param2) {
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 local0; 		// param1{32}
    __size32 local1; 		// param2{33}

    g_f = g_f - 1;
    local0 = param1;
    local1 = param2;
    if (g_f >= 0) {
        proc3();
        local0 = ecx;
        local1 = edx;
    }
    param1 = local0;
    param2 = local1;
    res += 13;
    return param1; /* WARNING: Also returning: edx := param2 */
}

// address: 0x8048594
void i() {
    res += 19;
    return;
}

// address: 0x80485d8
void k() {
    k_e = k_e - 1;
    if (k_e >= 0) {
        proc4();
    }
    res += 27;
    return;
}

