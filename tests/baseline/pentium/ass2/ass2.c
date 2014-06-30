union { union { union { int[] x377; unsigned int x378; } x253; __size32 x254; __size32 x252; __size32 x250; __size32 x248; __size32 x246; __size32 x244; __size32 x242; __size32 x240; unsigned int x230; __size32 x30; __size32 x28; __size32 x26; } x23; __size32 x24; } global2;
union { int x255; float x256; } global68;
unsigned char global73 = 1;
union { union { int[] x225; float x226; float x224; unsigned char x222; unsigned char x220; } x217; float x218; } global99;

void proc2();
void proc3();
void proc4(__size32 ??, int x, int y);
void proc5(int key, int x, int y);
void proc6(int width, int height);
void proc7();
void proc8();
void proc9(char param1[]);
void proc10(char param1[]);
__size32 *proc11(double param1, double param2, double param3, double param4, double param5, double param6);
int proc12(int param1);
__size32 *proc13(double param1, double param2, double param3, double param4, double param5, double param6);
__size32 *proc14(double param1, double param2, double param3, double param4, double param5, double param6);
__size32 *proc15(double param1, double param2, double param3, double param4, double param5, double param6);
double proc16(double param1, double param2, double param3, double param4, double param5, double param6);
void proc17(__size32 *param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5);
void proc18(__size32 *param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5);
void proc19(__size32 *param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5);
void proc20(__size32 *param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5);
void proc21(char param1[], union { GLfloat x19; __size32 x20; } param2, union { GLfloat x21; __size32 x22; } param3);
void proc22();
__size32 proc23(union { GLfloat x9; __size32 x10; } *param1);
void proc24(__size32 *param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6);

// address: 0x804b320
int main(int argc, char *argv[], char *envp[]) {
    int *local0; 		// m[esp - 28]

    glutInit(&argc, argv);
    glutInitDisplayMode(16);
    local0 = *argv;
    glutCreateWindow(local0);
    proc2();
    glutDisplayFunc(proc3);
    glutKeyboardFunc(proc4);
    glutSpecialFunc(proc5);
    glutReshapeFunc(proc6);
    glutMainLoop();
    return 0;
}

// address: 0x804a550
void proc2() {
    glClearColor(0, 0x3f000000, 0x3f333333, 0x3f800000);
    glEnable(0xb71);
    memset(0x804c280, 0, 500);
    memset(0x804ca50, 0, 4);
    return;
}

// address: 0x804ac50
void proc3() {
    __size32 eax; 		// r24
    int esp; 		// r28
    GLfloat local1; 		// m[esp - 28]

    glClear(0x4100);
    glLoadIdentity();
    if (*0x804c05c == 1) {
        glTranslatef(0, 0, (float)-100);
        eax = *(&global99 + 0x8c0);
        glTranslatef(0, 0, eax ^ 0x80000000);
    }
    local1 = *0x804c068;
    glRotatef(local1, 0x3f800000, 0, 0);
    local1 = *0x804c064;
    glRotatef(local1, 0, 0x3f800000, 0);
    proc7();
    esp = proc8();
    proc9(*(esp - 80));
    glFlush();
    *(__size32*)(esp - 16) = 0x804b76b;
    proc10(*(esp - 16));
    return;
}

// address: 0x804aeec
void proc4(__size32 ??, int x, int y) {
    unsigned char al; 		// r8
    __size32 eax; 		// r24
    int eax_1; 		// r24{314}
    __size32 local0; 		// m[esp + 4]
    double st; 		// r32

    eax = ((unsigned char) local0);
    if ((unsigned int)(eax - 27) <= 95) {
        switch(((unsigned char) local0)) {
        case 27:
            exit(0);
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
            al =  (*0x804c058 == 0) ? 1 : 0;
            *(unsigned char*)(&global99 + 0x8b8) = al;
            if (*0x804c058 == 0) {
                glShadeModel();
                break;
            } else {
                glShadeModel();
                break;
            }
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
        case 56:
        case 57:
        case 58:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
L11:
            st = *(&global99 + 0x8c8);
            *(float*)(&global99 + 0x8c8) = st + 2.;
            st = *(&global99 + 0x8c8);
            if (st > 90.) {
                *(__size32*)(&global99 + 0x8c8) = 0x42b40000;
                break;
            } else {
                break;
            }
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
        case 76:
        case 77:
        case 78:
        case 79:
        case 80:
L18:
            glutGet();
            glutGet();
            if (*0x804c05c != 1) {
                global68 = 1;
            } else {
                global68 = 0;
            }
            proc6(eax_1, eax);
            break;
        case 81:
        case 82:
        case 83:
        case 84:
        case 85:
        case 86:
        case 87:
L23:
            al =  (*0x804c059 == 0) ? 1 : 0;
            *(unsigned char*)(&global99 + 0x8b9) = al;
            if (*0x804c059 == 0) {
                glPolygonMode();
                break;
            } else {
                glPolygonMode();
                break;
            }
        case 88:
        case 89:
        case 90:
L26:
            st = *(&global99 + 0x8c8);
            *(float*)(&global99 + 0x8c8) = st - 2.;
            st = *(&global99 + 0x8c8);
            if (st < 0.) {
                *(__size32*)(&global99 + 0x8c8) = 0;
                break;
            } else {
                break;
            }
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
        case 96:
        case 97:
            goto L11;
        case 98:
        case 99:
        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
        case 105:
        case 106:
        case 107:
        case 108:
        case 109:
        case 110:
        case 111:
        case 112:
            goto L18;
        case 113:
        case 114:
        case 115:
        case 116:
        case 117:
        case 118:
        case 119:
            goto L23;
        case 120:
        case 121:
        case 122:
            goto L26;
        }
    }
    glutPostRedisplay();
    return;
}

// address: 0x804b098
void proc5(int key, int x, int y) {
    unsigned char al; 		// r8
    __size32 eax; 		// r24
    int eax_1; 		// r24{173}
    int eax_2; 		// r24{62}
    int eax_3; 		// r24{340}
    int eax_4; 		// r24{99}
    int eax_5; 		// r24{136}
    int eax_6; 		// r24{424}
    double st; 		// r32
    double st1; 		// r33
    double st2; 		// r34
    double st3; 		// r35
    double st4; 		// r36
    double st5; 		// r37

    if (key == 5) {
        eax_1 = proc11(st, st1, st2, st3, st4, st5);
        eax = proc12(eax_1);
        if (eax != -1) {
            *(__size32*)(&global2 + 2012)++;
        } else {
            free(eax_1);
        }
    } else {
        if (key > 5) {
            if (key == 101) {
                st = *(&global99 + 0x8c0);
                *(float*)(&global99 + 0x8c0) = st - 2.;
                if (*0x804c05c == 0) {
                    glutGet();
                    glutGet();
                    proc6(eax_3, eax);
                }
            } else {
                if (key > 101) {
                    if (key == 102) {
                        st = *(&global99 + 0x8c4);
                        *(float*)(&global99 + 0x8c4) = st - 2.;
                    } else {
                        if (key == 103) {
                            st = *(&global99 + 0x8c0);
                            *(float*)(&global99 + 0x8c0) = st + 2.;
                            if (*0x804c05c == 0) {
                                glutGet();
                                glutGet();
                                proc6(eax_6, eax);
                            }
                        }
                    }
                } else {
                    if (key == 100) {
                        st = *(&global99 + 0x8c4);
                        *(float*)(&global99 + 0x8c4) = st + 2.;
                    }
                }
            }
        } else {
            if (key == 2) {
                eax_2 = proc13(st, st1, st2, st3, st4, st5);
                eax = proc12(eax_2);
                if (eax != -1) {
                    *(__size32*)(&global2 + 2000)++;
                } else {
                    free(eax_2);
                }
            } else {
                if (key > 2) {
                    if (key == 3) {
                        eax_4 = proc14(st, st1, st2, st3, st4, st5);
                        eax = proc12(eax_4);
                        if (eax != -1) {
                            *(__size32*)(&global2 + 2004)++;
                        } else {
                            free(eax_4);
                        }
                    } else {
                        if (key == 4) {
                            eax_5 = proc15(st, st1, st2, st3, st4, st5);
                            eax = proc12(eax_5);
                            if (eax != -1) {
                                *(__size32*)(&global2 + 2008)++;
                            } else {
                                free(eax_5);
                            }
                        }
                    }
                } else {
                    if (key == 1) {
                        al =  (*0x804c05a == 0) ? 1 : 0;
                        global73 = al;
                    }
                }
            }
        }
    }
    glutPostRedisplay();
    return;
}

// address: 0x804ad04
void proc6(int width, int height) {
    GLdouble local0; 		// m[esp - 64]
    double st; 		// r32
    double st_1; 		// r32{16}
    double st_2; 		// r32{318}
    double st_3; 		// r32{536}

    st_1 = (float)width;
    st = (float)height;
    glViewport();
    glMatrixMode(0x1701);
    glLoadIdentity();
    if (global68 == 0) {
        st = st_1 / st;
        if (st > 1.) {
            st = (float)*0x804b4e0;
            st_3 = st * st_1 / st;
            st = (float)-100 * st_1 / st;
            glOrtho(st, -100, st_3, local0, (float)-100, -100);
        } else {
            st_2 = (float)-100 / st_1 / st;
            st = (float)*0x804b4e0;
            glOrtho((float)-100, -100, st, local0, st_2, -100);
        }
    } else {
        if (global68 == 1) {
            gluPerspective();
        } else {
            global68 = 1;
            gluPerspective();
        }
    }
    glMatrixMode(0x1700);
    return;
}

// address: 0x804ab20
void proc7() {
    double st; 		// r32
    double st_1; 		// r32{179}

    glColor3f(0x3f333333, 0x3f000000, 0);
    glBegin(7);
    glVertex3f((float)-100, 0, (float)-100);
    st = (float)*0x804b4e0;
    glVertex3f((float)-100, 0, st);
    st_1 = (float)*0x804b4e0;
    st = (float)*0x804b4e0;
    glVertex3f(st, 0, st_1);
    st = (float)*0x804b4e0;
    glVertex3f(st, 0, (float)-100);
    glEnd();
    return;
}

// address: 0x804aaa0
void proc8() {
    __size32 eax; 		// r24
    __size32 *ebp; 		// r29
    int esp; 		// r28
    int local0; 		// m[esp - 4]

    ebp = (esp - 4);
    glPushMatrix();
L6:
    while (*(ebp - 8) != 0) {
        *(__size32*)(esp - 16) = *(ebp - 8);
        esp = proc23(*(esp - 16)); /* Warning: also results in ebp */
        esp += 16;
        *(__size32*)(ebp - 4)++;
        if (*(ebp - 4) <= 499) {
            eax = *(ebp - 4);
            *(int*)(ebp - 8) = global2[eax];
            goto L6;
        }
        *(__size32*)(ebp - 8) = 0;
L6:
    }
    glPopMatrix();
    return;
}

// address: 0x804a940
void proc9(char param1[]) {
    glDisable(0xb71);
    sprintf(&param1, 0x804b700);
    glMatrixMode(0x1701);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 0, 0, 0x40240000, 0, 0);
    glMatrixMode(0x1700);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(0, 0x3f800000, 0);
    proc21(&param1, 0, 0x411ccccd);
    if (*0x804c05c != 1) {
        sprintf(&param1, 0x804b74a);
    } else {
        sprintf(&param1, 0x804b732);
    }
    proc21(&param1, 0, 0x41180000);
    if (*0x804c05a != 0) {
        proc22();
    }
    glPopMatrix();
    glMatrixMode(0x1701);
    glPopMatrix();
    glMatrixMode(0x1700);
    glEnable(0xb71);
    return;
}

// address: 0x804ac08
void proc10(char param1[]) {
    __size32 eax; 		// r24

    for(;;) {
        glGetError();
        if (eax == 0) {
            break;
        }
        gluErrorString(eax);
        printf("%s: %s\n", param1, eax);
    }
    return;
}

// address: 0x804a29c
__size32 *proc11(double param1, double param2, double param3, double param4, double param5, double param6) {
    long long eax; 		// r24
    __size32 *eax_1; 		// r24
    double st; 		// r32
    double st1; 		// r33
    double st2; 		// r34
    double st3; 		// r35
    double st4; 		// r36
    double st5; 		// r37
    double st6; 		// r38
    double st_1; 		// r32{71}

    calloc(1, 20);
    rand();
    tmpl = ( (eax < 0) ? -1 : 0) << 32 | eax;
    st_1 = proc16(param1, param2, param3, param4, param5, param6); /* Warning: also results in st1, st2, st3, st4, st5, st6 */
    st = proc16(st1, st2, st3, st4, st5, st6); /* Warning: also results in st1, st2, st3, st4, st5, st6 */
    st = fabs(st) + 2.;
    st = proc16(st1, st2, st3, st4, st5, st6);
    proc18(eax_1, st, st, st_1, (float)(tmpl % 360));
    return eax_1;
}

// address: 0x804a4e0
int proc12(int param1) {
    int local0; 		// m[esp - 8]

    local0 = 0;
    while (local0 <= 499) {
        if (global2[local0] == 0) {
            break;
        }
        local0++;
    }
    if (local0 > 499) {
        local0 = -1;
    } else {
        global2[local0] = param1;
    }
    return local0;
}

// address: 0x804a228
__size32 *proc13(double param1, double param2, double param3, double param4, double param5, double param6) {
    long long eax; 		// r24
    __size32 *eax_1; 		// r24
    double st; 		// r32
    double st1; 		// r33
    double st2; 		// r34
    double st3; 		// r35
    double st4; 		// r36
    double st5; 		// r37
    double st6; 		// r38
    double st_1; 		// r32{71}

    calloc(1, 20);
    rand();
    tmpl = ( (eax < 0) ? -1 : 0) << 32 | eax;
    st_1 = proc16(param1, param2, param3, param4, param5, param6); /* Warning: also results in st1, st2, st3, st4, st5, st6 */
    st = proc16(st1, st2, st3, st4, st5, st6);
    proc17(eax_1, st, 0x3f19999a, st_1, (float)(tmpl % 360));
    return eax_1;
}

// address: 0x804a330
__size32 *proc14(double param1, double param2, double param3, double param4, double param5, double param6) {
    long long eax; 		// r24
    __size32 *eax_1; 		// r24
    double st; 		// r32
    double st1; 		// r33
    double st2; 		// r34
    double st3; 		// r35
    double st4; 		// r36
    double st5; 		// r37
    double st6; 		// r38
    double st_1; 		// r32{71}

    calloc(1, 20);
    rand();
    tmpl = ( (eax < 0) ? -1 : 0) << 32 | eax;
    st_1 = proc16(param1, param2, param3, param4, param5, param6); /* Warning: also results in st1, st2, st3, st4, st5, st6 */
    st = proc16(st1, st2, st3, st4, st5, st6); /* Warning: also results in st1, st2, st3, st4, st5, st6 */
    st = fabs(st) + 6.;
    st = proc16(st1, st2, st3, st4, st5, st6);
    proc19(eax_1, st, st, st_1, (float)(tmpl % 360));
    return eax_1;
}

// address: 0x804a3c4
__size32 *proc15(double param1, double param2, double param3, double param4, double param5, double param6) {
    long long eax; 		// r24
    __size32 *eax_1; 		// r24
    double st; 		// r32
    double st1; 		// r33
    double st2; 		// r34
    double st3; 		// r35
    double st4; 		// r36
    double st5; 		// r37
    double st6; 		// r38
    double st_1; 		// r32{71}

    calloc(1, 20);
    rand();
    tmpl = ( (eax < 0) ? -1 : 0) << 32 | eax;
    st_1 = proc16(param1, param2, param3, param4, param5, param6); /* Warning: also results in st1, st2, st3, st4, st5, st6 */
    st = proc16(st1, st2, st3, st4, st5, st6);
    proc20(eax_1, st, 0x3ecccccd, st_1, (float)(tmpl % 360));
    return eax_1;
}

// address: 0x804a1f4
double proc16(double param1, double param2, double param3, double param4, double param5, double param6) {
    __size32 eax; 		// r24
    double st; 		// r32
    double st_1; 		// r32{54}

    rand();
    st_1 = (float)*0x804b4e0;
    st = (float)*0x804b4e0;
    st = (st_1 + st_1) * (float)eax / 2.14748e+09 - st;
    return st; /* WARNING: Also returning: st1 := param1, st2 := param2, st3 := param3, st4 := param4, st5 := param5, st6 := param6 */
}

// address: 0x804a154
void proc17(__size32 *param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5) {
    proc24(param1, param2, param3, param4, param5, 0x8049450);
    return;
}

// address: 0x804a17c
void proc18(__size32 *param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5) {
    proc24(param1, param2, param3, param4, param5, 0x804a138);
    return;
}

// address: 0x804a1a4
void proc19(__size32 *param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5) {
    proc24(param1, param2, param3, param4, param5, 0x80497dc);
    return;
}

// address: 0x804a1cc
void proc20(__size32 *param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5) {
    proc24(param1, param2, param3, param4, param5, 0x8049740);
    return;
}

// address: 0x804a5ac
void proc21(char param1[], union { GLfloat x19; __size32 x20; } param2, union { GLfloat x21; __size32 x22; } param3) {
    __size32 eax; 		// r24
    int eax_1; 		// r24
    unsigned int local0; 		// m[esp - 12]

    local0 = 0;
    glRasterPos2f(param2, param3);
    eax = local0 + param1;
    while (*eax != 0) {
        if (local0 > 999) {
            break;
        }
        eax = local0 + param1;
        eax_1 = (int) *eax;
        glutBitmapCharacter(0x804c240, eax_1);
        local0++;
        eax = local0 + param1;
    }
    return;
}

// address: 0x804a610
void proc22() {
    proc21(0x804b4e4, 0x40000000, 0x41100000);
    proc21(0x804b4fa, 0x40000000, 0x41000000);
    proc21(0x804b500, 0x40400000, 0x41000000);
    proc21(0x804b52d, 0x40000000, 0x40f00000);
    proc21(0x804b540, 0x40400000, 0x40f00000);
    proc21(0x804b56d, 0x40000000, 0x40e00000);
    proc21(0x804b578, 0x40400000, 0x40e00000);
    proc21(0x804b58b, 0x40000000, 0x40d00000);
    proc21(0x804b58d, 0x40400000, 0x40d00000);
    proc21(0x804b5a8, 0x40000000, 0x40c00000);
    proc21(0x804b5ab, 0x40400000, 0x40c00000);
    proc21(0x804b5be, 0x40000000, 0x40b00000);
    proc21(0x804b5c1, 0x40400000, 0x40b00000);
    proc21(0x804b5d3, 0x40000000, 0x40a00000);
    proc21(0x804b5d6, 0x40400000, 0x40a00000);
    proc21(0x804b5eb, 0x40000000, 0x40900000);
    proc21(0x804b5ee, 0x40400000, 0x40900000);
    proc21(0x804b604, 0x40000000, 0x40800000);
    proc21(0x804b607, 0x40400000, 0x40800000);
    proc21(0x804b619, 0x40000000, 0x40400000);
    proc21(0x804b620, 0x40400000, 0x40400000);
    proc21(0x804b64a, 0x40000000, 0x40200000);
    proc21(0x804b660, 0x40400000, 0x40200000);
    proc21(0x804b68a, 0x40000000, 0x40000000);
    proc21(0x804b68f, 0x40400000, 0x40000000);
    proc21(0x804b6a2, 0x40000000, 0x3fc00000);
    proc21(0x804b6a8, 0x40400000, 0x3fc00000);
    proc21(0x804b6c2, 0x40000000, 0x3f800000);
    proc21(0x804b6c4, 0x40400000, 0x3f800000);
    proc21(0x804b6df, 0x40000000, 0x3f000000);
    proc21(0x804b6e1, 0x40400000, 0x3f000000);
    return;
}

// address: 0x804a438
__size32 proc23(union { GLfloat x9; __size32 x10; } *param1) {
    __size32 eax; 		// r24
    __size32 ebp; 		// r29
    __size32 ecx; 		// r25
    __size32 edx; 		// r26
    int esp; 		// r28
    union { GLfloat x13; __size32 x14; } local0; 		// m[esp - 20]
    union { GLfloat x11; __size32 x12; } local1; 		// m[esp - 24]
    union { GLfloat x9; union { GLfloat x9; __size32 x10; } * x10; } local2; 		// m[esp - 28]

    glPushMatrix();
    local0 = *(param1 + 8);
    local1 = *(param1 + 4);
    local2 = *param1;
    glTranslatef(local2, local1, local0);
    local2 = *(param1 + 12);
    glRotatef(local2, 0, 0x3f800000, 0);
    eax = *(param1 + 16);
    (**(param1 + 16))(pc, param1, 0, 0x3f800000, 0, ebp, param1, eax, ecx, edx, esp - 4, SUBFLAGS32((esp - 12), 12, esp - 24), esp - 24 == 0, esp - 12 < 12);
    glPopMatrix();
    ebp = *ebp;
    return ebp;
}

// address: 0x8048ffc
void proc24(__size32 *param1, __size32 param2, __size32 param3, __size32 param4, __size32 param5, __size32 param6) {
    *(__size32*)param1 = param2;
    *(__size32*)(param1 + 4) = param3;
    *(__size32*)(param1 + 8) = param4;
    *(__size32*)(param1 + 12) = param5;
    *(__size32*)(param1 + 16) = param6;
    return;
}

