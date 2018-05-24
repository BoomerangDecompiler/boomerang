int h_i = 3;
int res;
int g_f = 3;
int f_g = 3;
int j_k = 3;
int c_j = 3;
int l_b = 3;
int e_c = 3;
int b_c = 3;
int d_e = 3;
int k_e = 3;
int c_d = 3;
int c_l = 3;
int c_h = 3;
int c_f = 3;
int h_i = 3;
int res;
int g_f = 3;
int f_g = 3;
int j_k = 3;
int c_j = 3;
int l_b = 3;
int e_c = 3;
int b_c = 3;
int d_e = 3;
int k_e = 3;
int c_d = 3;
int c_l = 3;
int c_h = 3;
int c_f = 3;
int main(int argc, char *argv[]);
__size32 b(__size32 param1, __size32 param2);
__size32 c(int param1, __size32 param2, __size32 param3);
__size32 d(__size32 param1, __size32 param2);
void f();
void h();
__size32 j(__size32 param1, __size32 param2);
__size32 l(__size32 param1, __size32 param2);
__size32 e(__size32 param1, __size32 param2);
void g();
void i();
__size32 k(__size32 param1, __size32 param2);

/** address: 0x0804837c */
int main(int argc, char *argv[])
{
    int ecx; 		// r25
    int edx; 		// r26

    ecx = b(55, 99); /* Warning: also results in edx */
    printf("ecx is %d, edx is %d\n", edx, ecx);
    printf("res is %d\n", res);
    return 0;
}

/** address: 0x080483d6 */
__size32 b(__size32 param1, __size32 param2)
{
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 edx_1; 		// r26{0}
    __size32 local4; 		// param1{0}
    __size32 local5; 		// param2{0}

    b_c = b_c - 1;
    local4 = param1;
    local5 = param2;
    if (b_c >= 0) {
        ecx = c(param1, param2, param1); /* Warning: also results in edx_1 */
        edx = ecx;
        ecx = edx_1;
        local4 = ecx;
        local5 = edx;
    }
    param1 = local4;
    param2 = local5;
    res += 2;
    return param1; /* WARNING: Also returning: edx := param2 */
}

/** address: 0x08048408 */
__size32 c(int param1, __size32 param2, __size32 param3)
{
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 edx_1; 		// r26{0}
    __size32 edx_2; 		// r26{0}
    __size32 edx_3; 		// r26{0}
    __size32 esp; 		// r28
    int local0; 		// m[esp - 4]
    __size32 local3; 		// param2{0}
    __size32 local4; 		// param3{0}
    __size32 local5; 		// ecx{0}
    __size32 local6; 		// edx{0}

    esp = &param1;
    c_d = c_d - 1;
    local3 = param2;
    local4 = param3;
    if (c_d >= 0) {
        ecx = d(param3, param2); /* Warning: also results in edx_1, esp */
        edx = ecx;
        ecx = edx_1;
        local3 = ecx;
        local4 = edx;
    }
    param2 = local3;
    param3 = local4;
    c_f = c_f - 1;
    local5 = param2;
    local6 = param3;
    if (c_f >= 0) {
        f();
    }
    c_h = c_h - 1;
    if (c_h >= 0) {
        h();
    }
    c_j = c_j - 1;
    if (c_j >= 0) {
        ecx = j(param3, param2); /* Warning: also results in edx_2 */
        edx = ecx;
        ecx = edx_2;
        local5 = ecx;
        local6 = edx;
    }
    ecx = local5;
    edx = local6;
    c_l = c_l - 1;
    if (c_l >= 0) {
        ecx = l(edx, ecx); /* Warning: also results in edx_3 */
        edx = ecx;
        ecx = edx_3;
    }
    res += 3;
    return ecx; /* WARNING: Also returning: edx := edx */
}

/** address: 0x080484a6 */
__size32 d(__size32 param1, __size32 param2)
{
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 edx_1; 		// r26{0}
    __size32 local3; 		// param1{0}
    __size32 local4; 		// param2{0}

    d_e = d_e - 1;
    local3 = param1;
    local4 = param2;
    if (d_e >= 0) {
        ecx = e(param2, param1); /* Warning: also results in edx */
        edx_1 = ecx;
        ecx = edx;
        local3 = ecx;
        local4 = edx_1;
    }
    param1 = local3;
    param2 = local4;
    res += 5;
    return param1; /* WARNING: Also returning: edx := param2 */
}

/** address: 0x0804850a */
void f()
{
    f_g = f_g - 1;
    if (f_g >= 0) {
        g();
    }
    res += 11;
    return;
}

/** address: 0x08048566 */
void h()
{
    h_i = h_i - 1;
    if (h_i >= 0) {
        i();
    }
    res += 17;
    return;
}

/** address: 0x080485a6 */
__size32 j(__size32 param1, __size32 param2)
{
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 edx_1; 		// r26{0}
    __size32 local3; 		// param1{0}
    __size32 local4; 		// param2{0}

    j_k = j_k - 1;
    local3 = param1;
    local4 = param2;
    if (j_k >= 0) {
        ecx = k(param2, param1); /* Warning: also results in edx_1 */
        edx = ecx;
        ecx = edx_1;
        local3 = ecx;
        local4 = edx;
    }
    param1 = local3;
    param2 = local4;
    res += 23;
    return param1; /* WARNING: Also returning: edx := param2 */
}

/** address: 0x0804860b */
__size32 l(__size32 param1, __size32 param2)
{
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 edx_1; 		// r26{0}
    __size32 local3; 		// param1{0}
    __size32 local4; 		// param2{0}

    l_b = l_b - 1;
    local3 = param1;
    local4 = param2;
    if (l_b >= 0) {
        ecx = b(param2, param1); /* Warning: also results in edx_1 */
        edx = ecx;
        ecx = edx_1;
        local3 = ecx;
        local4 = edx;
    }
    param1 = local3;
    param2 = local4;
    res += 29;
    return param1; /* WARNING: Also returning: edx := param2 */
}

/** address: 0x080484d8 */
__size32 e(__size32 param1, __size32 param2)
{
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    __size32 edx_1; 		// r26{0}
    __size32 local4; 		// param1{0}
    __size32 local5; 		// param2{0}

    e_c = e_c - 1;
    local4 = param1;
    local5 = param2;
    if (e_c >= 0) {
        ecx = c(param1, param2, param1); /* Warning: also results in edx_1 */
        edx = ecx;
        ecx = edx_1;
        local4 = ecx;
        local5 = edx;
    }
    param1 = local4;
    param2 = local5;
    res += 7;
    return param1; /* WARNING: Also returning: edx := param2 */
}

/** address: 0x08048538 */
void g()
{
    g_f = g_f - 1;
    if (g_f >= 0) {
        f();
    }
    res += 13;
    return;
}

/** address: 0x08048594 */
void i()
{
    res += 19;
    return;
}

/** address: 0x080485d8 */
__size32 k(__size32 param1, __size32 param2)
{
    __size32 ecx; 		// r25
    __size32 ecx_1; 		// r25{0}
    __size32 edx; 		// r26
    __size32 local3; 		// param1{0}
    __size32 local4; 		// param2{0}

    k_e = k_e - 1;
    local3 = param1;
    local4 = param2;
    if (k_e >= 0) {
        ecx_1 = e(param2, param1); /* Warning: also results in edx */
        ecx = edx;
        edx = ecx_1 - 1;
        local3 = ecx;
        local4 = edx;
    }
    param1 = local3;
    param2 = local4;
    res += 27;
    return param1; /* WARNING: Also returning: edx := param2 */
}

